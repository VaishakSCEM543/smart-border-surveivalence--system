// ============================================================
// STAGE 8A — ESP-NOW TRANSMITTER (Bot 1)
// Bot 1 | ESP32
// ============================================================
// PURPOSE:
//   After Stage 7 works, this adds ESP-NOW wireless transmission.
//   Bot 1 runs the full sequence (move + scan), then transmits
//   the obstacle data packet to Bot 2 wirelessly.
//
// SETUP:
//   1. First, your FRIEND must get Bot 2's MAC address:
//      → Upload the helper sketch: stage8_get_mac.ino to Bot 2
//      → Read the MAC from Serial Monitor
//      → Tell you the MAC address (e.g. AA:BB:CC:DD:EE:FF)
//   2. Paste that MAC address into BOT2_MAC below
//   3. Upload this to Bot 1
//
// ⚠ IMPORTANT:
//   Both ESP32s must be on the same Wi-Fi channel (default = 1).
//   ESP-NOW works WITHOUT a Wi-Fi router — it's peer-to-peer.
//
// PASS CRITERIA:
//   ✓ Bot 1 runs, scans, prints packet locally
//   ✓ Bot 2 Serial Monitor shows received packet
// ============================================================

#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

// ============================================================
// ★ PASTE BOT 2's MAC ADDRESS HERE ★
// Format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
// ============================================================
uint8_t BOT2_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // ← replace this!

// --- Motor Pins ---
#define IN1  27
#define IN2  26
#define IN3  25
#define IN4  33
#define ENA  14
#define ENB  32

// --- Encoder Pins ---
#define LEFT_A   18
#define LEFT_B   19
#define RIGHT_A  21
#define RIGHT_B  22

// --- Sensor Pins ---
#define TRIG_PIN   5
#define ECHO_PIN   4
#define SERVO_PIN  13

// ============================================================
#define COUNTS_PER_CM   66.0f   // ← from Stage 2
#define TARGET_CM       30.0f
#define TARGET_COUNTS   (long)(TARGET_CM * COUNTS_PER_CM)
#define DRIVE_SPEED     130
#define SYNC_GAIN       0.4f
// ============================================================

const int SCAN_ANGLES[]  = {0, 30, 60, 90, 120, 150, 180};
const int NUM_ANGLES     = 7;
#define SETTLE_TIME_MS   300
#define MAX_RANGE_CM     200.0f

// ============================================================
// Data packet structure — MUST match Bot 2's definition exactly
// ============================================================
typedef struct ObstaclePacket {
  uint8_t  botID;            // Always 1
  float    distanceTraveled; // cm
  int      angles[7];        // scan angles (degrees)
  float    distances[7];     // distances at each angle (cm), -1 = clear
} ObstaclePacket;

// ============================================================

volatile long leftCount  = 0;
volatile long rightCount = 0;

Servo myServo;
float scanDistances[7];

// ============================================================

void IRAM_ATTR leftISR()
{
  if (digitalRead(LEFT_A) == digitalRead(LEFT_B)) leftCount++;
  else leftCount--;
}

void IRAM_ATTR rightISR()
{
  if (digitalRead(RIGHT_A) == digitalRead(RIGHT_B)) rightCount++;
  else rightCount--;
}

void leftMotor(int speed)
{
  speed = constrain(speed, -255, 255);
  if (speed > 0)      { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); }
  else if (speed < 0) { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  }
  else                { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  }
  analogWrite(ENA, abs(speed));
}

void rightMotor(int speed)
{
  speed = constrain(speed, -255, 255);
  if (speed > 0)      { digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
  else if (speed < 0) { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  }
  else                { digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  }
  analogWrite(ENB, abs(speed));
}

void stopMotors()
{
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

float readDistance()
{
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  if (dur == 0) return -1.0f;
  return (dur * 0.0343f) / 2.0f;
}

// ============================================================

void onDataSent(const uint8_t *mac, esp_now_send_status_t status)
{
  Serial.print(">>> ESP-NOW send: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS ✓" : "FAILED ✗");
}

// ============================================================

void driveStraight()
{
  Serial.println(">>> PHASE 1: Driving 30 cm...");
  noInterrupts(); leftCount = 0; rightCount = 0; interrupts();

  while (true)
  {
    noInterrupts(); long L = leftCount; long R = rightCount; interrupts();
    long avg = (L + R) / 2;
    if (avg >= TARGET_COUNTS) break;

    long err = L - R;
    leftMotor (constrain(DRIVE_SPEED - (int)(SYNC_GAIN * err), 60, 255));
    rightMotor(constrain(DRIVE_SPEED + (int)(SYNC_GAIN * err), 60, 255));
  }

  stopMotors();
  noInterrupts(); long L = leftCount; long R = rightCount; interrupts();
  float dist = ((float)(L + R) / 2.0f) / COUNTS_PER_CM;
  Serial.print(">>> STOPPED at ~"); Serial.print(dist, 1); Serial.println(" cm");
  delay(500);
}

void runScan()
{
  Serial.println(">>> PHASE 2: Scanning...");
  for (int i = 0; i < NUM_ANGLES; i++)
  {
    myServo.write(SCAN_ANGLES[i]);
    delay(SETTLE_TIME_MS);
    scanDistances[i] = readDistance();
    Serial.print("  "); Serial.print(SCAN_ANGLES[i]); Serial.print("° → ");
    if (scanDistances[i] < 0 || scanDistances[i] > MAX_RANGE_CM) Serial.println("CLEAR");
    else { Serial.print(scanDistances[i], 1); Serial.println(" cm"); }
  }
  myServo.write(90); delay(300);
}

void sendPacket()
{
  ObstaclePacket pkt;
  pkt.botID            = 1;
  pkt.distanceTraveled = TARGET_CM;

  for (int i = 0; i < NUM_ANGLES; i++)
  {
    pkt.angles[i]    = SCAN_ANGLES[i];
    pkt.distances[i] = (scanDistances[i] > MAX_RANGE_CM) ? -1.0f : scanDistances[i];
  }

  esp_now_send(BOT2_MAC, (uint8_t*)&pkt, sizeof(pkt));

  Serial.println();
  Serial.println(">>> PHASE 3: Packet sent via ESP-NOW");
}

// ============================================================

void setup()
{
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(LEFT_A,  INPUT_PULLUP); pinMode(LEFT_B,  INPUT_PULLUP);
  pinMode(RIGHT_A, INPUT_PULLUP); pinMode(RIGHT_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_A),  leftISR,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_A), rightISR, CHANGE);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(90);

  stopMotors();

  // --- ESP-NOW setup ---
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW init FAILED. Check MAC and power.");
    while (true) delay(1000);
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BOT2_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  STAGE 8 — FULL BOT 1 + ESP-NOW TX");
  Serial.println("=========================================");
  Serial.print  ("  My MAC : "); Serial.println(WiFi.macAddress());
  Serial.print  ("  Bot2 MAC: ");
  for (int i = 0; i < 6; i++)
  {
    if (BOT2_MAC[i] < 0x10) Serial.print("0");
    Serial.print(BOT2_MAC[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  Serial.println();
  Serial.println("Send 'G' to go.");
  Serial.println();
}

// ============================================================

void loop()
{
  if (Serial.available())
  {
    char cmd = Serial.read();
    if (cmd == 'G' || cmd == 'g')
    {
      driveStraight();
      runScan();
      sendPacket();
      Serial.println("Complete. Send 'G' to run again.");
    }
  }
}
