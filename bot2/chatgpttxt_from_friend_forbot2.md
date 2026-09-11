This is the perfect next step.

Since Bot 1 uses **N20 micro gear motors with encoders**, the hardware behaves a bit differently than Bot 2's BO motors.

Here are two important hardware notes before we look at the code:

1. **Lower Speed Value:** N20s are highly efficient and have gearboxes. The `AUTO_SPEED` might need to be tweaked (I've set it to 150) so Bot 1 doesn't drive significantly faster or slower than Bot 2.
2. **Encoder Voltage Warning:** The Hall effect encoders on the back of those N20 motors have their own `VCC` and `GND` pins. **You must power the encoder VCC from the ESP32's 3.3V pin.** If you power the encoder from 5V or 6V, the signal wires will send 5V+ back into the ESP32 and instantly fry the GPIO pins!

### The Bot 1 Code

I have included the exact same Web Server and ESP-NOW framework so it integrates perfectly with your beautiful new dashboard.

I have also added the **Interrupt setup for your encoders**. Even though the basic obstacle avoidance still uses simple timers (to match Bot 2), the ESP32 will now silently count the exact motor rotations in the background, preparing Bot 1 for precise PID control later.

**Things to change before uploading:**

1. Your Wi-Fi SSID and Password.
2. Paste **Bot 2's MAC Address** at the top.
3. Update the Encoder GPIO pins (lines 43-46) to match wherever you plugged them in. (I defaulted to input-only pins 34, 35, 36, 39).

```cpp
/********************************************************************
   ESP32 SWARM BOT 1 (N20 ENCODERS + UNICAST + WEB API)
********************************************************************/

#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <ESP32Servo.h>

/**************** NETWORK SETTINGS ****************/
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// OPTION B: Paste BOT 2's MAC Address here!
uint8_t bot2Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

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
const int TRIG = 5; const int ECHO = 18; const int SERVO_PIN = 4; 
Servo scanServo;

// N20 ENCODER PINS (Input-only pins are great for this)
const int ENCA_LEFT = 34; const int ENCB_LEFT = 35;
const int ENCA_RIGHT = 36; const int ENCB_RIGHT = 39;

volatile long leftEncoderTicks = 0;
volatile long rightEncoderTicks = 0;

/**************** SETTINGS ****************/
const int AUTO_SPEED = 150;         // Tuned for N20 150RPM motors
const float OBSTACLE_LIMIT = 0.30f; 
const int TURN_TIME_MS = 500;

unsigned long last_auto = 0;

/**************** ENCODER INTERRUPTS ****************/
void IRAM_ATTR leftEncoderISR() {
  if (digitalRead(ENCA_LEFT) == digitalRead(ENCB_LEFT)) leftEncoderTicks++;
  else leftEncoderTicks--;
}

void IRAM_ATTR rightEncoderISR() {
  if (digitalRead(ENCA_RIGHT) == digitalRead(ENCB_RIGHT)) rightEncoderTicks++;
  else rightEncoderTicks--;
}

/**************** HTTP API ENDPOINT ****************/
void handleDataRequest() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  char jsonStr[150];
  // Formatted exactly how your custom HTML dashboard expects it
  snprintf(jsonStr, sizeof(jsonStr), 
    "{\"bot_id\": 1, \"local_distance\": %.2f, \"local_blocked\": %s, \"peer_blocked\": %s}", 
    myData.front_distance, 
    myData.obstacle_front ? "true" : "false", 
    peerIsBlocked ? "true" : "false"
  );
  
  server.send(200, "application/json", jsonStr);
}

/**************** ESP-NOW CALLBACKS ****************/
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&peerData, incomingData, sizeof(peerData));
  if(peerData.bot_id == 2) { // Listen specifically for Bot 2
    peerIsBlocked = peerData.obstacle_front;
    lastPeerMessageTime = millis();
  }
}

/**************** MOTOR & SENSOR LOGIC ****************/
void setMotorPWM(int left_pwm, int right_pwm) {
  left_pwm = constrain(left_pwm, -255, 255); right_pwm = constrain(right_pwm, -255, 255);
  
  if (left_pwm > 0) { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); }
  else if (left_pwm < 0) { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); }
  else { digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); }
  
  if (right_pwm > 0) { digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); }
  else if (right_pwm < 0) { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
  else { digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); }
  
  analogWrite(ENA, abs(left_pwm)); analogWrite(ENB, abs(right_pwm));
}

void stopMotors() { setMotorPWM(0, 0); }

float readDistance() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 30000); delay(60); 
  if (duration == 0) return 4.0f; 
  float distance = (duration * 0.034f / 2.0f) / 100.0f; 
  if (distance <= 0.02f || distance > 4.0f) return 4.0f;
  return distance;
}

/**************** SWARM BEHAVIOR ****************/
void autonomousControl() {
  scanServo.write(90); delay(60); 
  float front = readDistance();
  myData.obstacle_front = (front < OBSTACLE_LIMIT);
  myData.front_distance = front;
  
  myData.bot_id = 1; // I am Bot 1
  esp_now_send(bot2Address, (uint8_t *) &myData, sizeof(myData));

  if (millis() - lastPeerMessageTime > 2000) { peerIsBlocked = false; }

  if (!myData.obstacle_front) {
    if(peerIsBlocked) setMotorPWM(AUTO_SPEED / 2, AUTO_SPEED / 2); 
    else setMotorPWM(AUTO_SPEED, AUTO_SPEED); 
  } else {
    stopMotors(); delay(120);
    scanServo.write(30); delay(220); float right_dist = readDistance();
    scanServo.write(150); delay(220); float left_dist = readDistance();
    scanServo.write(90); delay(80);

    if (left_dist > right_dist) setMotorPWM(-AUTO_SPEED, AUTO_SPEED); 
    else setMotorPWM(AUTO_SPEED, -AUTO_SPEED); 
    
    delay(TURN_TIME_MS); stopMotors(); delay(80);
  }
}

/**************** SETUP ****************/
void setup() {
  Serial.begin(115200);

  // Motor & Sensor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT); pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  
  // Encoder Pins
  pinMode(ENCA_LEFT, INPUT_PULLUP); pinMode(ENCB_LEFT, INPUT_PULLUP);
  pinMode(ENCA_RIGHT, INPUT_PULLUP); pinMode(ENCB_RIGHT, INPUT_PULLUP);
  
  // Attach Encoder Interrupts
  attachInterrupt(digitalPinToInterrupt(ENCA_LEFT), leftEncoderISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCA_RIGHT), rightEncoderISR, RISING);

  scanServo.setPeriodHertz(50); scanServo.attach(SERVO_PIN, 500, 2400); scanServo.write(90);
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
  esp_now_register_send_cb(OnDataSent);
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

  if (millis() - last_auto > 150) {
    autonomousControl();
    last_auto = millis();
  }
}

```
use similar structure for bot 1 code, this is a refernce code for bot 1 i genearted, you haev the generate ip and mac