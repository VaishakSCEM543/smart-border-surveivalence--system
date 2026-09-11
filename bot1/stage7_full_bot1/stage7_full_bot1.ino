/********************************************************************
   ESP32 SWARM BOT 1 (FINAL)
   FEATURES:
   - ESP-NOW Point-to-Point (Option B) - Sending to Bot 2
   - Wi-Fi Station Mode Coexistence & Web Server (Port 80)
   - Synchronized Straight Driving (N20 Encoders)
   - HC-SR04 + Servo Obstacle Avoidance
********************************************************************/

#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <ESP32Servo.h>

/**************** NETWORK SETTINGS ****************/
const char* ssid = "realme GT NEO 3T";
const char* password = "qwerty4u";

// MAC ADDRESS OF DUMMY BOT 2 (Extracted from serial monitor!)
uint8_t bot2Address[] = {0xC0, 0xCD, 0xD6, 0x8E, 0x3E, 0xD8}; 

WebServer server(80); 

/**************** ESP-NOW SWARM DATA ****************/
typedef struct swarm_message {
  uint8_t bot_id;        
  bool obstacle_front;   
  float front_distance;  
} swarm_message;

swarm_message myData;
swarm_message peerData;

unsigned long lastPeerMessageTime = 0; 
bool peerIsBlocked = false;

/**************** HARDWARE PINS ****************/
// L298N Motor Pins
const int IN1 = 27; const int IN2 = 26; const int ENA = 14; 
const int IN3 = 25; const int IN4 = 33; const int ENB = 32; 

// Sensor Pins
const int TRIG = 5; 
const int ECHO = 4; // Using your voltage divider pin
const int SERVO_PIN = 12; // Adjusted to your latest test

Servo scanServo;

// N20 ENCODER PINS (From our working Stage 3)
const int LEFT_A = 21; const int LEFT_B = 22;
const int RIGHT_A = 18; const int RIGHT_B = 19;

volatile long leftCount = 0;
volatile long rightCount = 0;

/**************** SETTINGS ****************/
const int BASE_SPEED = 140;         
const int MIN_SPEED = 85; // Prevents USB stall
const int MAX_SPEED = 255;
const float KP = 0.5f;

const float OBSTACLE_LIMIT = 30.0f; // cm
const int TURN_TIME_MS = 500;

unsigned long last_auto = 0;
unsigned long lastControlTime = 0;

enum BotState { STATE_DRIVE, STATE_SCAN, STATE_TURN };
BotState currentState = STATE_DRIVE;

/**************** ENCODER INTERRUPTS ****************/
void IRAM_ATTR leftISR() {
  if (digitalRead(LEFT_A) == digitalRead(LEFT_B)) leftCount--;
  else leftCount++;
}

void IRAM_ATTR rightISR() {
  if (digitalRead(RIGHT_A) == digitalRead(RIGHT_B)) rightCount++;
  else rightCount--;
}

/**************** HTTP API ENDPOINT ****************/
void handleDataRequest() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  char jsonStr[150];
  snprintf(jsonStr, sizeof(jsonStr), 
    "{\"bot_id\": 1, \"local_distance\": %.2f, \"local_blocked\": %s, \"peer_blocked\": %s}", 
    myData.front_distance, 
    myData.obstacle_front ? "true" : "false", 
    peerIsBlocked ? "true" : "false"
  );
  server.send(200, "application/json", jsonStr);
}

/**************** ESP-NOW CALLBACKS ****************/
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  memcpy(&peerData, incomingData, sizeof(peerData));
  if(peerData.bot_id == 2) { 
    peerIsBlocked = peerData.obstacle_front;
    lastPeerMessageTime = millis();
  }
}

/**************** MOTOR & SENSOR LOGIC ****************/
void leftMotor(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0)      { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); }
  else if (speed < 0) { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  }
  else                { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  }
  analogWrite(ENA, abs(speed));
}

void rightMotor(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0)      { digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
  else if (speed < 0) { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  }
  else                { digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  }
  analogWrite(ENB, abs(speed));
}

void stopMotors() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

float readDistance() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 25000); // 25ms timeout
  if (duration == 0) return 400.0f; 
  float distance = (duration * 0.0343f) / 2.0f; 
  if (distance <= 2.0f || distance > 400.0f) return 400.0f;
  return distance;
}

/**************** SWARM BEHAVIOR ****************/
void autonomousControl() {
  if (millis() - lastPeerMessageTime > 2000) { peerIsBlocked = false; }

  if (currentState == STATE_DRIVE) {
    // 1. Read Distance
    float front = readDistance();
    myData.obstacle_front = (front < OBSTACLE_LIMIT);
    myData.front_distance = front;
    
    // Broadcast status to Bot 2
    myData.bot_id = 1; 
    esp_now_send(bot2Address, (uint8_t *) &myData, sizeof(myData));

    if (myData.obstacle_front) {
      stopMotors();
      currentState = STATE_SCAN;
      return;
    }

    // 2. Synchronized Straight Driving (20ms loop)
    if (millis() - lastControlTime >= 20) {
      lastControlTime = millis();
      
      noInterrupts();
      long L = abs(leftCount);
      long R = abs(rightCount);
      interrupts();

      long error = L - R;
      int correction = (int)(KP * error);

      int currentBase = peerIsBlocked ? (BASE_SPEED - 30) : BASE_SPEED; // Slow down if peer is blocked
      
      int leftSpd  = constrain(currentBase - correction, MIN_SPEED, MAX_SPEED);
      int rightSpd = constrain(currentBase + correction, MIN_SPEED, MAX_SPEED);

      leftMotor(leftSpd);
      rightMotor(rightSpd);
    }
  } 
  else if (currentState == STATE_SCAN) {
    delay(120);
    scanServo.write(30); delay(400); 
    float right_dist = readDistance();
    
    scanServo.write(150); delay(400); 
    float left_dist = readDistance();
    
    scanServo.write(90); delay(200);

    currentState = STATE_TURN;
    if (left_dist > right_dist) {
      leftMotor(-BASE_SPEED); rightMotor(BASE_SPEED);
    } else {
      leftMotor(BASE_SPEED); rightMotor(-BASE_SPEED);
    }
    
    delay(TURN_TIME_MS);
    stopMotors();
    delay(100);
    
    // Reset encoders for next straight drive
    noInterrupts(); leftCount = 0; rightCount = 0; interrupts();
    currentState = STATE_DRIVE;
  }
}

/**************** SETUP ****************/
void setup() {
  Serial.begin(115200);

  // Motor & Sensor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT); pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  
  // Encoder Pins
  pinMode(LEFT_A, INPUT_PULLUP); pinMode(LEFT_B, INPUT_PULLUP);
  pinMode(RIGHT_A, INPUT_PULLUP); pinMode(RIGHT_B, INPUT_PULLUP);
  
  // Attach Encoder Interrupts
  attachInterrupt(digitalPinToInterrupt(LEFT_A), leftISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_A), rightISR, CHANGE);

  ESP32PWM::allocateTimer(0);
  scanServo.setPeriodHertz(50); 
  scanServo.attach(SERVO_PIN, 500, 2400); 
  scanServo.write(90);
  stopMotors();

  // 1. Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("\n\n========================================");
  Serial.println("NETWORK CONNECTED!");
  Serial.print("-> BOT 1 MAC ADDRESS: ");
  Serial.println(WiFi.macAddress());
  Serial.print("-> BOT 1 IP ADDRESS (FOR WEBPAGE): ");
  Serial.println(WiFi.localIP());
  Serial.println("========================================\n");

  // 2. Initialize ESP-NOW
  int32_t channel = WiFi.channel();
  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);

  // Register Bot 2
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, bot2Address, 6);
  peerInfo.channel = channel; 
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // 3. Start Web Server
  server.on("/data", HTTP_GET, handleDataRequest);
  server.begin();
}

/**************** MAIN LOOP ****************/
void loop() {
  server.handleClient(); 
  autonomousControl();
}
