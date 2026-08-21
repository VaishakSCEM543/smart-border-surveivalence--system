/*
  BOT 1 — UDP Sender (IP Based)
  Sensors: Dual N20 encoders, GPS6MV2, VL53L0X+Servo, HLK-LD2410C
*/

#include <Wire.h>
#include <VL53L0X.h>
#include <ESP32Servo.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <MyLD2410.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// ─── Wi-Fi & UDP Settings ─────────────────────────────────
const char* ssid     = "Ashwin";
const char* password = "12345678";
const char* targetIP = "192.168.4.1"; // Bot 2's AP address
const int   udpPort  = 4210;
WiFiUDP udp;

// ─── Motor A (Left) ───────────────────────────────────────
#define ENC_A1  18
#define ENC_B1  19
#define IN1     26
#define IN2     27
#define ENA     14

// ─── Motor B (Right) ──────────────────────────────────────
#define ENC_A2  35
#define ENC_B2  34
#define IN3     25
#define IN4     33
#define ENB     32

// ─── Servo + VL53L0X ──────────────────────────────────────
#define SERVO_PIN  13
#define SDA_PIN    21
#define SCL_PIN    22

// ─── GPS (Serial2) ────────────────────────────────────────
#define GPS_RX  16
#define GPS_TX  17
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

// ─── LD2410C (Serial1) ────────────────────────────────────
#define LD2410_RX   4   
#define LD2410_TX   5   // Unplug when uploading!
#define LD2410_OUT  23  
HardwareSerial ld2410Serial(1);
MyLD2410 radar(ld2410Serial);

// ─── Encoder & Motor Specs ────────────────────────────────
const int   PPR          = 3;
const int   GEAR_RATIO   = 30;
const float WHEEL_DIA_MM = 65.0;
const int   PWM_FREQ     = 20000;
const int   PWM_RES      = 8;
const int   DRIVE_SPEED  = 255;

volatile long pulseA = 0, pulseB = 0;
VL53L0X tof;
Servo   radarServo;

const int RADAR_POINTS = 19;
struct RadarPoint { int angle; int distance; bool objectDetected; };
RadarPoint radarData[RADAR_POINTS];

float gpsLat = 0.0, gpsLng = 0.0, gpsSpeed = 0.0;
int   gpsSats = 0;
bool  ld_presence = false, ld_moving = false, ld_stationary = false, ld_outState = false;
int   ld_moveDist = 0, ld_statDist = 0, ld_moveSig = 0, ld_statSig = 0;
int   frontDistance = 1000;
float totalDistMM = 0.0;
long  lastPA = 0, lastPB = 0;

enum BotMode { IDLE, EXPLORING, AVOIDING };
BotMode botMode = IDLE;

const int OBSTACLE_MM = 250;
const int HUMAN_MIN   = 200;
const int HUMAN_MAX   = 1500;

// ─── Packet Structure ─────────────────────────────────────
typedef struct {
  float distA_mm, distB_mm, totalDist_mm;
  long  pA, pB;
  float gpsLat, gpsLng, gpsSpeed;
  int   gpsSats;
  bool  ld_presence, ld_moving, ld_stationary, ld_outState;
  int   ld_moveDist, ld_statDist, ld_moveSig, ld_statSig;
  int   frontDist;
  int   sweepDist[19];
  bool  sweepHit[19];
  char  botMode[12];
  uint32_t timestamp;
} BotPacket;

BotPacket txPacket;

// ─────────────────────────────────────────────────────────
void IRAM_ATTR encoderISR_A() { if (digitalRead(ENC_B1) == HIGH) pulseA++; else pulseA--; }
void IRAM_ATTR encoderISR_B() { if (digitalRead(ENC_B2) == HIGH) pulseB++; else pulseB--; }
float toMM(long p) { return ((float)p / (float)(PPR * GEAR_RATIO)) * PI * WHEEL_DIA_MM; }

void updateOdometry() {
  long dA = abs(pulseA) - abs(lastPA);
  long dB = abs(pulseB) - abs(lastPB);
  if (dA > 0 || dB > 0) totalDistMM += toMM((dA + dB) / 2);
  lastPA = abs(pulseA); lastPB = abs(pulseB);
}

void motorA_forward(int s)  { digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  ledcWrite(ENA,s); }
void motorA_backward(int s) { digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); ledcWrite(ENA,s); }
void motorA_stop()          { digitalWrite(IN1,LOW);  digitalWrite(IN2,LOW);  ledcWrite(ENA,0); }
void motorB_forward(int s)  { digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  ledcWrite(ENB,s); }
void motorB_backward(int s) { digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); ledcWrite(ENB,s); }
void motorB_stop()          { digitalWrite(IN3,LOW);  digitalWrite(IN4,LOW);  ledcWrite(ENB,0); }
void stopAll()              { motorA_stop(); motorB_stop(); }
void driveForward(int s)    { motorA_forward(s); motorB_forward(s); }
void driveBackward(int s)   { motorA_backward(s); motorB_backward(s); }
void turnLeft(int s)        { motorA_backward(s); motorB_forward(s); }
void turnRight(int s)       { motorA_forward(s); motorB_backward(s); }

int readToF() {
  int d = tof.readRangeSingleMillimeters();
  return tof.timeoutOccurred() ? 8190 : d;
}

void readLD2410() {
  radar.check();
  ld_outState   = digitalRead(LD2410_OUT);
  ld_presence   = radar.presenceDetected();
  ld_moving     = radar.movingTargetDetected();
  ld_stationary = radar.stationaryTargetDetected();
  ld_moveDist   = ld_moving ? radar.movingTargetDistance() : 0;
  ld_statDist   = ld_stationary ? radar.stationaryTargetDistance() : 0;
  ld_moveSig    = ld_moving ? radar.movingTargetSignal() : 0;
  ld_statSig    = ld_stationary ? radar.stationaryTargetSignal() : 0;
}

void radarSweep() {
  for (int i = 0; i < RADAR_POINTS; i++) {
    int angle = i * 10;
    radarServo.write(angle);
    delay(60);
    int d = readToF();
    radarData[i] = { angle, d, (d >= HUMAN_MIN && d <= HUMAN_MAX) };
    if (angle == 90) frontDistance = d; 
  }
  radarServo.write(90); 
}

void avoidObstacle() {
  stopAll(); delay(200);
  driveBackward(DRIVE_SPEED); delay(400); 
  stopAll(); delay(100);
  
  radarServo.write(150); delay(300); int lD = readToF();
  radarServo.write(30);  delay(300); int rD = readToF();
  radarServo.write(90);  
  
  if (lD > rD) turnLeft(DRIVE_SPEED); else turnRight(DRIVE_SPEED);
  delay(400); stopAll();
}

void feedGPS() {
  unsigned long t = millis();
  while (millis() - t < 80)
    while (gpsSerial.available()) gps.encode(gpsSerial.read());
  if (gps.location.isValid()) {
    gpsLat = gps.location.lat(); gpsLng = gps.location.lng();
    gpsSpeed = gps.speed.kmph(); gpsSats = gps.satellites.value();
  }
}

// ─── UDP Transmission ─────────────────────────────────────
void sendUDPData() {
  if (WiFi.status() == WL_CONNECTED) {
    txPacket.distA_mm    = toMM(pulseA);
    txPacket.distB_mm    = toMM(pulseB);
    txPacket.totalDist_mm= totalDistMM;
    txPacket.pA          = pulseA;
    txPacket.pB          = pulseB;
    txPacket.gpsLat      = gpsLat;
    txPacket.gpsLng      = gpsLng;
    txPacket.gpsSpeed    = gpsSpeed;
    txPacket.gpsSats     = gpsSats;
    txPacket.ld_outState = ld_outState;
    txPacket.ld_presence = ld_presence;
    txPacket.ld_moving   = ld_moving;
    txPacket.ld_stationary = ld_stationary;
    txPacket.ld_moveDist = ld_moveDist;
    txPacket.ld_statDist = ld_statDist;
    txPacket.ld_moveSig  = ld_moveSig;
    txPacket.ld_statSig  = ld_statSig;
    txPacket.frontDist   = frontDistance;
    txPacket.timestamp   = millis();

    const char* mStr = botMode==EXPLORING?"EXPLORING":botMode==AVOIDING?"AVOIDING":"IDLE";
    strncpy(txPacket.botMode, mStr, sizeof(txPacket.botMode));

    for (int i = 0; i < RADAR_POINTS; i++) {
      txPacket.sweepDist[i] = radarData[i].distance;
      txPacket.sweepHit[i]  = radarData[i].objectDetected;
    }

    udp.beginPacket(targetIP, udpPort);
    udp.write((uint8_t*)&txPacket, sizeof(txPacket));
    udp.endPacket();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  
  pinMode(LD2410_OUT, INPUT);
  ld2410Serial.begin(256000, SERIAL_8N1, LD2410_RX, LD2410_TX);
  delay(500);
  if (!radar.begin()) Serial.println("WARNING: LD2410C not found!");

  Wire.begin(SDA_PIN, SCL_PIN);
  tof.init(); 
  tof.setTimeout(40); 
  tof.startContinuous();

  radarServo.attach(SERVO_PIN);
  radarServo.write(90); 

  pinMode(ENC_A1, INPUT_PULLUP); pinMode(ENC_B1, INPUT_PULLUP);
  pinMode(ENC_A2, INPUT);        pinMode(ENC_B2, INPUT); 
  attachInterrupt(digitalPinToInterrupt(ENC_A1), encoderISR_A, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_A2), encoderISR_B, RISING);

  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT); ledcAttach(ENA,PWM_FREQ,PWM_RES);
  pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT); ledcAttach(ENB,PWM_FREQ,PWM_RES);
  stopAll();

  // Connect to Bot 2 via Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Bot 2");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! My IP: " + WiFi.localIP().toString());
  udp.begin(udpPort);

  radarSweep();
  botMode = EXPLORING;
}

void loop() {
  feedGPS();
  readLD2410();
  updateOdometry();
  frontDistance = readToF();

  if (frontDistance < OBSTACLE_MM || ld_outState || ld_presence) {
    botMode = AVOIDING;
    stopAll();
    radarSweep();
    avoidObstacle();
    botMode = EXPLORING;
  } else {
    botMode = EXPLORING;
    driveForward(DRIVE_SPEED);

    static unsigned long lastSweep = 0;
    if (millis() - lastSweep > 1500) {
      stopAll();
      radarSweep();
      lastSweep = millis();
      driveForward(DRIVE_SPEED);
    }
  }

  // Send UDP data 
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 300) {
    sendUDPData();
    lastSend = millis();
  }

  delay(20);
}