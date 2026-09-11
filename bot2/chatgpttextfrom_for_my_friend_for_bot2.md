This is a fantastic architectural choice. Moving to **Option B (Unicast)** makes the swarm communication highly reliable, and adding a **Web Server** transforms your project into a professional IoT system.

To do this, the ESP32s will perform "coexistence"—they will use **ESP-NOW to talk to each other instantly**, while simultaneously connecting to your local **Wi-Fi router to serve data to your webpage**.

Here is exactly how the architecture works and everything you need to build your frontend webpage.

### 1. The Network Architecture

Because ESP-NOW and standard Wi-Fi share the same physical antenna, **both bots must be connected to the exact same Wi-Fi router**.

* The router determines the Wi-Fi Channel (e.g., Channel 6).
* We will force ESP-NOW to also use that exact same channel so the bots can talk peer-to-peer while remaining connected to the router.
* You will browse to `http://<BOT_IP>/data` from your computer to fetch the live swarm data.

### 2. What Your Webpage Needs to Do

You don't need a complex backend. Your ESP32s will act as the backend (API). You just need an HTML file with some simple JavaScript that polls the bots.

I will build a `/data` endpoint into the bot's code. When your webpage makes a `GET` request to `http://<BOT_IP>/data`, the bot will reply with a JSON object that looks exactly like this:

```json
{
  "bot_id": 2,
  "local_distance": 0.45,
  "local_blocked": false,
  "peer_blocked": true
}

```

**Crucial Webpage Detail (CORS):** I have added a CORS header (`Access-Control-Allow-Origin: *`) to the ESP32 code below. Without this, your web browser would block your custom webpage from reading the data!

### 3. The JavaScript Fetch Logic (For Your Webpage)

In your custom HTML file, you will use `setInterval()` to pull this data every 500 milliseconds. It will look something like this:

```javascript
// In your webpage's script.js
const bot2_IP = "192.168.1.100"; // You will get this from the Serial Monitor

setInterval(() => {
    fetch(`http://${bot2_IP}/data`)
        .then(response => response.json())
        .then(data => {
            console.log(data);
            // Update your HTML elements here:
            // document.getElementById('bot2-dist').innerText = data.local_distance;
        })
        .catch(error => console.error("Error fetching Bot 2:", error));
}, 500); // Polls every half second

```

### 4. The Option B Swarm Code (Bot 2)

Here is the updated code for **Bot 2**.

**Before you upload, you must change 3 things at the top of the code:**

1. Your Wi-Fi `SSID`.
2. Your Wi-Fi `PASSWORD`.
3. The exact `MAC Address` of Bot 1 (using the short scanner script I mentioned earlier).

```cpp
/********************************************************************
   ESP32 SWARM BOT 2 (OPTION B: UNICAST + WEB API)
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
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// OPTION B: Put BOT 1's MAC Address here!
uint8_t bot1Address[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; 

WebServer server(80); // Create Web Server on port 80

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
const float OBSTACLE_LIMIT = 0.30f; 
const int TURN_TIME_MS = 500;

unsigned long last_auto = 0;

/**************** HTTP API ENDPOINT ****************/
void handleDataRequest() {
  // CORS Header so your external webpage can read the data
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
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Silent transmission
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&peerData, incomingData, sizeof(peerData));
  if(peerData.bot_id == 1) { 
    peerIsBlocked = peerData.obstacle_front;
    lastPeerMessageTime = millis();
  }
}

/**************** MOTOR & SENSOR LOGIC ****************/
// (Standard motor code remains unchanged)
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
  
  // Send specifically to Bot 1 (Unicast)
  myData.bot_id = 2; 
  esp_now_send(bot1Address, (uint8_t *) &myData, sizeof(myData));

  if (millis() - lastPeerMessageTime > 2000) { peerIsBlocked = false; }

  if (!myData.obstacle_front) {
    if(peerIsBlocked) setMotorPWM(AUTO_SPEED / 2, AUTO_SPEED / 2); // Teammate blocked
    else setMotorPWM(AUTO_SPEED, AUTO_SPEED); // Both clear
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

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT); pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  
  scanServo.setPeriodHertz(50); scanServo.attach(SERVO_PIN, 500, 2400); scanServo.write(90);
  stopMotors();

  // 1. Connect to Wi-Fi Network First
  WiFi.mode(WIFI_STA);
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("YOUR BOT 2 IP ADDRESS IS: "); 
  Serial.println(WiFi.localIP()); // COPY THIS IP FOR YOUR WEBPAGE

  // 2. Initialize ESP-NOW on the Router's Channel
  int32_t channel = WiFi.channel();
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW Init Failed"); return; }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register Bot 1 as Peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, bot1Address, 6);
  peerInfo.channel = channel; // Must match Wi-Fi channel
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){ Serial.println("Failed to add Bot 1"); return; }

  // 3. Start Web Server
  server.on("/data", HTTP_GET, handleDataRequest);
  server.begin();
  Serial.println("Web Server Started");
}

/**************** MAIN LOOP ****************/
void loop() {
  // Always handle web requests as fast as possible
  server.handleClient();

  if (millis() - last_auto > 150) {
    autonomousControl();
    last_auto = millis();
  }
}

```
bro refer this to make the second bot