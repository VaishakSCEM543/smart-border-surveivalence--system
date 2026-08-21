/*
  BOT 2 — DIRECT IP CLIENT + ISOLATED SERVO TIMERS + WEB SERVER
*/
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================================
// PASTE BOT 1's IP ADDRESS HERE
const char* BOT1_IP = "10.142.196.34"; // <--- UPDATE THIS!
// ==========================================

const char* ssid = "Ashwin";
const char* password = "12345678";

// --- MAP TRACKING ---
float botX = 0.0, botY = 0.0, botHeading = 90.0; 
unsigned long lastCalcTime = 0;
const float LINEAR_SPEED = 40.0, TURN_SPEED = 110.0; 
int lastMeasuredDist = 100, lastServoAngle = 115;
String b1_json_data = "{}"; 

// --- WEB SERVER ---
WebServer server(80);

// --- OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
enum DriveState { STOPPED, FWD, REV, LFT, RGT, DANGER };
DriveState currentDispState = STOPPED;

// --- PINS & CONSTANTS ---
const int IN1 = 27, IN2 = 26, ENA = 13, IN3 = 25, IN4 = 33, ENB = 32;
const int TRIG_PIN = 5, ECHO_PIN = 18, SERVO_PIN = 23;
Servo scanServo;

// --- YOUR SPECIFIC ANGLES ---
const int SERVO_CENTER = 115;
const int SERVO_LEFT   = SERVO_CENTER + 55;  // 170
const int SERVO_RIGHT  = SERVO_CENTER - 55;  // 60

const int MOTOR_SPEED_A = 135, MOTOR_SPEED_B = 135;  
const int OBSTACLE_DIST = 30, TURN_TIME = 500, REVERSE_TIME = 900;  

void fetchBot1Data() {
  HTTPClient http;
  String url = "http://" + String(BOT1_IP) + "/telemetry";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) { b1_json_data = http.getString(); } 
  else { b1_json_data = "{\"error\":\"Timeout/Offline\"}"; }
  http.end();
}

void handleMapData() {
  String json = "{";
  json += "\"x\":" + String(botX) + ",\"y\":" + String(botY) + ",\"heading\":" + String(botHeading) + ",";
  json += "\"servoAngle\":" + String(lastServoAngle) + ",\"dist\":" + String(lastMeasuredDist) + ",";
  json += "\"bot1\":" + b1_json_data; 
  json += "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// --- ODOMETRY & DISPLAY LOGIC ---
void updateOdometry() {
  unsigned long now = millis(); float dt = (now - lastCalcTime) / 1000.0; lastCalcTime = now;
  if (currentDispState == FWD) { botX += cos(botHeading * PI / 180.0) * LINEAR_SPEED * dt; botY += sin(botHeading * PI / 180.0) * LINEAR_SPEED * dt; } 
  else if (currentDispState == REV) { botX -= cos(botHeading * PI / 180.0) * LINEAR_SPEED * dt; botY -= sin(botHeading * PI / 180.0) * LINEAR_SPEED * dt; } 
  else if (currentDispState == LFT) { botHeading += TURN_SPEED * dt; } 
  else if (currentDispState == RGT) { botHeading -= TURN_SPEED * dt; }
}

void updateDisplay(DriveState newState) {
  if (currentDispState == newState) return; 
  currentDispState = newState; display.clearDisplay();
  if (newState == FWD) { display.fillTriangle(32, 10, 5, 50, 59, 50, SSD1306_WHITE); display.fillRect(22, 50, 20, 60, SSD1306_WHITE); } 
  else if (newState == REV) { display.fillTriangle(32, 118, 5, 78, 59, 78, SSD1306_WHITE); display.fillRect(22, 18, 20, 60, SSD1306_WHITE); } 
  else if (newState == LFT) { display.fillTriangle(5, 64, 35, 34, 35, 94, SSD1306_WHITE); display.fillRect(35, 54, 25, 20, SSD1306_WHITE); } 
  else if (newState == RGT) { display.fillTriangle(59, 64, 29, 34, 29, 94, SSD1306_WHITE); display.fillRect(4, 54, 25, 20, SSD1306_WHITE); } 
  else if (newState == DANGER) {
    display.fillTriangle(32, 10, 0, 110, 63, 110, SSD1306_WHITE);    
    display.fillTriangle(32, 22, 8, 104, 55, 104, SSD1306_BLACK);    
    display.fillRect(28, 45, 8, 35, SSD1306_WHITE);                  
    display.fillRect(28, 88, 8, 8, SSD1306_WHITE);                   
  }
  display.display();
}

void moveForward() { updateDisplay(FWD); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); ledcWrite(ENA, MOTOR_SPEED_A); ledcWrite(ENB, MOTOR_SPEED_B); }
void stopMotors() { updateDisplay(STOPPED); ledcWrite(ENA, 0); ledcWrite(ENB, 0); delay(50); }
void reverseBot(int duration) { updateDisplay(REV); digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); ledcWrite(ENA, MOTOR_SPEED_A); ledcWrite(ENB, MOTOR_SPEED_B); unsigned long start = millis(); while(millis() - start < duration) { server.handleClient(); updateOdometry(); delay(10); } stopMotors(); delay(200); }
void turnLeft() { updateDisplay(LFT); digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); ledcWrite(ENA, MOTOR_SPEED_A); ledcWrite(ENB, MOTOR_SPEED_B); unsigned long start = millis(); while(millis() - start < TURN_TIME) { server.handleClient(); updateOdometry(); delay(10); } stopMotors(); delay(100); }
void turnRight() { updateDisplay(RGT); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); ledcWrite(ENA, MOTOR_SPEED_A); ledcWrite(ENB, MOTOR_SPEED_B); unsigned long start = millis(); while(millis() - start < TURN_TIME) { server.handleClient(); updateOdometry(); delay(10); } stopMotors(); delay(100); }

long getDistance() {
  long total = 0; int valid = 0;
  for (int i = 0; i < 3; i++) { 
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2); digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10); digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 25000);
    long dist = (duration == 0) ? 100 : duration * 0.034 / 2;
    if (dist >= 0 && dist < 200) { total += dist; valid++; } delay(5);
  }
  return lastMeasuredDist = (valid == 0) ? 100 : total / valid;
}

long getDistanceAt(int angle) { scanServo.write(angle); lastServoAngle = angle; delay(400); return getDistance(); }

void escapeContact() {
  updateDisplay(DANGER); digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); ledcWrite(ENA, 160); ledcWrite(ENB, 160);
  unsigned long start = millis(); while(millis() - start < 1200) { server.handleClient(); updateOdometry(); delay(10); } stopMotors(); delay(300);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { display.setRotation(3); display.clearDisplay(); display.setTextSize(2); display.setTextColor(SSD1306_WHITE); display.setCursor(4, 50); display.println("BOOTING"); display.display(); }

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); 
  ledcAttach(ENA, 1000, 8); ledcAttach(ENB, 1000, 8);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);

  // --- THE FIX: ISOLATING THE SERVO TIMERS ---
  ESP32PWM::allocateTimer(2); 
  ESP32PWM::allocateTimer(3);
  scanServo.setPeriodHertz(50); 
  scanServo.attach(SERVO_PIN, 544, 2400); 
  scanServo.write(SERVO_CENTER); 
  delay(500);
  
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print("\nBOT 2 IP: "); Serial.println(WiFi.localIP());

  server.on("/data", HTTP_OPTIONS, []() { server.sendHeader("Access-Control-Allow-Origin", "*"); server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS"); server.sendHeader("Access-Control-Allow-Headers", "Content-Type"); server.send(204); });
  server.on("/data", HTTP_GET, handleMapData);
  server.begin(); lastCalcTime = millis();
}

void loop() {
  server.handleClient(); updateOdometry();      
  scanServo.write(SERVO_CENTER); lastServoAngle = SERVO_CENTER;
  long dist = getDistance();

  static unsigned long lastFetch = 0;
  if (millis() - lastFetch > 500) {
    fetchBot1Data();
    
    Serial.println("\n======================================");
    Serial.println("         [ BOT 1 TELEMETRY ]          ");
    Serial.println(b1_json_data); 
    Serial.println("         [ BOT 2 LOCAL DATA ]         ");
    Serial.printf("X: %.2f | Y: %.2f | Head: %.1f | Dist: %d\n", botX, botY, botHeading, lastMeasuredDist);
    Serial.println("======================================");
    
    lastFetch = millis();
  }

  if (dist <= 5) {
    stopMotors(); escapeContact();
    long leftDist  = getDistanceAt(SERVO_LEFT); long rightDist = getDistanceAt(SERVO_RIGHT);
    if (leftDist <= OBSTACLE_DIST && rightDist <= OBSTACLE_DIST) { reverseBot(1200); turnLeft(); } 
    else if (leftDist > rightDist) { turnLeft(); } else { turnRight(); }
    return;
  }

  if (dist <= OBSTACLE_DIST) {
    stopMotors(); updateDisplay(DANGER); delay(200);
    long leftDist  = getDistanceAt(SERVO_LEFT); long rightDist = getDistanceAt(SERVO_RIGHT);
    int attempts = 0;
    while (leftDist <= OBSTACLE_DIST && rightDist <= OBSTACLE_DIST && attempts < 3) {
      reverseBot(REVERSE_TIME); updateDisplay(DANGER); 
      leftDist  = getDistanceAt(SERVO_LEFT); rightDist = getDistanceAt(SERVO_RIGHT); attempts++;
    }
    if (leftDist > rightDist) { turnLeft(); } else { turnRight(); }
    return;
  }

  moveForward();
}