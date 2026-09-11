/********************************************************************
   ESP32 SWARM BOT 2 (DUMMY RECEIVER FOR TESTING)
   FEATURES:
   - ESP-NOW Point-to-Point (Option B)
   - Wi-Fi Station Mode Coexistence
   - HTTP Web Server on Port 80 (/data endpoint)
********************************************************************/

#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <ESP32Servo.h>

/**************** NETWORK SETTINGS ****************/
const char* ssid = "realme GT NEO 3T";
const char* password = "qwerty4u";

// MAC ADDRESS OF BOT 1 (Just extracted!)
uint8_t bot1Address[] = {0x00, 0x4B, 0x12, 0xEE, 0xC9, 0xCC}; 

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

/**************** HARDWARE PINS (DC MOTORS & SERVO) ****************/
const int IN1 = 27; const int IN2 = 26; const int ENA = 14; 
const int IN3 = 25; const int IN4 = 33; const int ENB = 32; 
const int TRIG = 5; const int ECHO = 18; const int SERVO_PIN = 4; 

Servo scanServo;

/**************** SETTINGS ****************/
const int AUTO_SPEED = 180;         
const float OBSTACLE_LIMIT = 30.0f; 
const int TURN_TIME_MS = 500;

unsigned long last_auto = 0;

/**************** HTTP API ENDPOINT ****************/
void handleDataRequest() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  char jsonStr[150];
  snprintf(jsonStr, sizeof(jsonStr), 
    "{\"bot_id\": 2, \"local_distance\": %.2f, \"local_blocked\": %s, \"peer_blocked\": %s}", 
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
  if(peerData.bot_id == 1) { 
    peerIsBlocked = peerData.obstacle_front;
    lastPeerMessageTime = millis();
    
    // Print to Serial so you know it's working!
    Serial.println("\n[ESP-NOW] Received data from Bot 1!");
    Serial.print("Bot 1 Blocked: "); Serial.println(peerIsBlocked ? "YES" : "NO");
    Serial.print("Bot 1 Distance: "); Serial.println(peerData.front_distance);
  }
}

/**************** MOTOR & SENSOR LOGIC ****************/
void setMotorPWM(int left_pwm, int right_pwm) {
  // Dummy doesn't have motors, so do nothing.
}

void stopMotors() { setMotorPWM(0, 0); }

float readDistance() {
  // Dummy doesn't have sensors. Just return a safe distance.
  return 100.0f;
}

/**************** SWARM BEHAVIOR ****************/
void autonomousControl() {
  // Dummy behavior
  myData.obstacle_front = false;
  myData.front_distance = 100.0f;
  myData.bot_id = 2; 
  esp_now_send(bot1Address, (uint8_t *) &myData, sizeof(myData));

  if (millis() - lastPeerMessageTime > 2000) { peerIsBlocked = false; }
}

/**************** SETUP ****************/
void setup() {
  Serial.begin(115200);

  // 1. Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n\n========================================");
  Serial.println("DUMMY BOT 2 CONNECTED!");
  Serial.print("-> DUMMY BOT 2 MAC ADDRESS: ");
  Serial.println(WiFi.macAddress());
  Serial.print("-> DUMMY BOT 2 IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  Serial.println("========================================\n");

  // 2. Initialize ESP-NOW
  int32_t channel = WiFi.channel();
  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);

  // Register Bot 1
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, bot1Address, 6);
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
