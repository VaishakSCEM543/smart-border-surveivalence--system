/*
  BOT 1 — FULL SENSOR LOGIC + DIRECT IP WEB SERVER
*/
#include <Wire.h>
#include <VL53L0X.h>
#include <ESP32Servo.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <MyLD2410.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Abcd";
const char* password = "12345678";

WebServer server(80);

// --- PINS ---
#define ENC_A1 18
#define ENC_B1 19
#define IN1 26
#define IN2 27
#define ENA 14
#define ENC_A2 35
#define ENC_B2 34
#define IN3 25
#define IN4 33
#define ENB 32
#define SERVO_PIN 13
#define SDA_PIN 21
#define SCL_PIN 22
#define GPS_RX 16
#define GPS_TX 17
#define LD2410_RX 4
#define LD2410_TX 5 // UNPLUG TO UPLOAD

// --- CONSTANTS ---
const int PPR = 3; const int GEAR_RATIO = 30; const float WHEEL_DIA_MM = 65.0f;
const int PWM_FREQ = 20000; const int PWM_RES = 8;
const int DRIVE_SPEED = 200; const int TURN_SPEED_PWM = 200;
const int OBSTACLE_MM = 250; const int TURN_MS = 450; const int REVERSE_MS = 500;
const int RADAR_POINTS = 19;
const unsigned long SWEEP_INTERVAL = 1800; 

// --- STATE ---
volatile long pulseA = 0, pulseB = 0;
long lastPA = 0, lastPB = 0;
float totalDistMM = 0.0f;
float gpsLat = 0.0f, gpsLng = 0.0f;
int gpsSats = 0, frontDistance = 1000, lastRadarAngle = 90, lastRadarDist = 1000;
bool ld_presence = false, ld_moving = false;
unsigned long lastSweep = 0, lastLog = 0;

HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
HardwareSerial ld2410Serial(1);
MyLD2410 radar(ld2410Serial);
VL53L0X tof;
Servo radarServo;

// --- API ENDPOINT ---
void handleTelemetry() {
  String json = "{";
  json += "\"totalDistMM\":" + String(totalDistMM) + ",";
  json += "\"frontDistance\":" + String(frontDistance) + ",";
  json += "\"gpsLat\":" + String(gpsLat, 6) + ",";
  json += "\"gpsLng\":" + String(gpsLng, 6) + ",";
  json += "\"gpsSats\":" + String(gpsSats) + ",";
  json += "\"ld_presence\":" + String(ld_presence) + ",";
  json += "\"ld_moving\":" + String(ld_moving) + ",";
  json += "\"radarAngle\":" + String(lastRadarAngle) + ",";
  json += "\"radarDist\":" + String(lastRadarDist);
  json += "}";
  server.send(200, "application/json", json);
}

// --- ENCODERS & ODOMETRY ---
void IRAM_ATTR encoderISR_A() { if (digitalRead(ENC_B1) == HIGH) pulseA++; else pulseA--; }
void IRAM_ATTR encoderISR_B() { if (digitalRead(ENC_B2) == HIGH) pulseB++; else pulseB--; }
float pulsesToMM(long p) { return ((float)p / (float)(PPR * GEAR_RATIO)) * PI * WHEEL_DIA_MM; }
void updateOdometry() {
  long dA = abs(pulseA) - abs(lastPA); long dB = abs(pulseB) - abs(lastPB);
  if (dA > 0 || dB > 0) totalDistMM += pulsesToMM((dA + dB) / 2);
  lastPA = abs(pulseA); lastPB = abs(pulseB);
}

// --- MOTORS ---
void mA(int dir, int spd) { 
  if (dir > 0) { digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW); }
  else if (dir < 0) { digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH); }
  else { digitalWrite(IN1,LOW); digitalWrite(IN2,LOW); }
  ledcWrite(ENA, (dir==0)?0:spd);
}
void mB(int dir, int spd) {
  if (dir > 0) { digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW); }
  else if (dir < 0) { digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH); }
  else { digitalWrite(IN3,LOW); digitalWrite(IN4,LOW); }
  ledcWrite(ENB, (dir==0)?0:spd);
}
void stopAll() { mA(0,0); mB(0,0); }
void driveForward() { mA(1,DRIVE_SPEED); mB(1,DRIVE_SPEED); }
void driveBackward() { mA(-1,DRIVE_SPEED); mB(-1,DRIVE_SPEED); }
void pivotLeft() { mA(-1,TURN_SPEED_PWM); mB(1,TURN_SPEED_PWM); }
void pivotRight() { mA(1,TURN_SPEED_PWM); mB(-1,TURN_SPEED_PWM); }

void timedMotion(void(*fn)(), unsigned long ms) {
  fn();
  unsigned long start = millis();
  while (millis() - start < ms) {
    updateOdometry();
    server.handleClient(); // KEEP SERVER ALIVE DURING TURNS
    delay(8);
  }
  stopAll(); delay(80);
}

// --- SENSORS ---
int readToF() {
  int d = tof.readRangeSingleMillimeters();
  return (tof.timeoutOccurred() || d > 8000) ? 8000 : d;
}
int readToFAt(int angle) {
  radarServo.write(angle); lastRadarAngle = angle; delay(350);
  int d = readToF(); lastRadarDist = d;
  return d;
}

void readLD2410() { radar.check(); ld_presence = radar.presenceDetected(); ld_moving = radar.movingTargetDetected(); }
void feedGPS() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());
  if (gps.location.isValid()) { gpsLat = gps.location.lat(); gpsLng = gps.location.lng(); gpsSats = gps.satellites.value(); }
}

bool radarSweep() {
  bool blocked = false;
  Serial.println(">> EXECUTING RADAR SWEEP <<");
  for (int i = 0; i < RADAR_POINTS; i++) {
    int angle = i * 10;
    int d = readToFAt(angle);
    if (angle == 90) frontDistance = d;
    if (d < OBSTACLE_MM) blocked = true;
    feedGPS(); readLD2410(); updateOdometry(); server.handleClient();
  }
  radarServo.write(90); lastRadarAngle = 90; delay(200);
  return blocked;
}

void avoidObstacle() {
  stopAll(); delay(150);
  timedMotion(driveBackward, REVERSE_MS);
  int lD = readToFAt(150); int rD = readToFAt(30);
  radarServo.write(90);
  if (lD > rD) timedMotion(pivotLeft, TURN_MS); else timedMotion(pivotRight, TURN_MS);
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  ld2410Serial.begin(256000, SERIAL_8N1, LD2410_RX, LD2410_TX);
  radar.begin();
  Wire.begin(SDA_PIN, SCL_PIN);
  tof.init(); tof.setTimeout(50); tof.setMeasurementTimingBudget(33000); tof.startContinuous();
  radarServo.attach(SERVO_PIN); radarServo.write(90);

  pinMode(ENC_A1, INPUT_PULLUP); pinMode(ENC_B1, INPUT_PULLUP);
  pinMode(ENC_A2, INPUT); pinMode(ENC_B2, INPUT); 
  attachInterrupt(digitalPinToInterrupt(ENC_A1), encoderISR_A, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_A2), encoderISR_B, RISING);

  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT); ledcAttach(ENA, PWM_FREQ, PWM_RES);
  pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT); ledcAttach(ENB, PWM_FREQ, PWM_RES);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(300); }
  
  Serial.println("\n===========================");
  Serial.print("BOT 1 IP: "); Serial.println(WiFi.localIP());
  Serial.println("===========================");

  server.on("/telemetry", handleTelemetry);
  server.begin();

  radarSweep(); lastSweep = millis();
}

void loop() {
  server.handleClient(); // Handle incoming IP requests from Bot 2
  feedGPS(); readLD2410(); updateOdometry();
  frontDistance = readToF();

  if (millis() - lastLog > 500) {
    Serial.printf("[B1 LOG] ToF:%4dmm | Radar:%d | Sats:%d | Lat:%.5f | Lng:%.5f\n", 
                  frontDistance, ld_presence, gpsSats, gpsLat, gpsLng);
    lastLog = millis();
  }

  if (ld_presence || (frontDistance < OBSTACLE_MM && frontDistance > 0)) {
    Serial.println("!!! OBSTACLE OR HUMAN DETECTED !!!");
    stopAll(); bool blocked = radarSweep();
    if (blocked) avoidObstacle();
    lastSweep = millis(); driveForward(); return;
  }

  if (millis() - lastSweep >= SWEEP_INTERVAL) {
    stopAll(); bool blocked = radarSweep();
    lastSweep = millis();
    if (blocked) avoidObstacle();
    driveForward(); return;
  }

  driveForward(); delay(10);
}