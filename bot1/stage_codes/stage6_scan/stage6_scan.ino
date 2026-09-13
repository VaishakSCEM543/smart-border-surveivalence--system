// ============================================================
// STAGE 6 — COMBINED SCAN (Servo + HC-SR04)
// Bot 1 | ESP32 + SG90 + HC-SR04
// ============================================================
// PURPOSE:
//   Rotate servo through 7 angles, take a distance reading
//   at each angle. Print a scan table to Serial Monitor.
//   This is the obstacle mapping sub-system.
//
// CONNECTIONS:
//   TRIG  → GPIO 5
//   ECHO  → GPIO 4  (via 1kΩ/2kΩ voltage divider)
//   Servo → GPIO 13 (external 5V)
//
// HOW TO USE:
//   1. Upload, open Serial Monitor at 115200
//   2. Send 'S' to run a single scan
//   3. Check that distances change when you place objects at different angles
//
// PASS CRITERIA:
//   ✓ Servo moves to each position before reading
//   ✓ Distances are plausible (not all 0 or -1)
//   ✓ Obstacle in one direction shows as shorter distance at that angle
// ============================================================

#include <ESP32Servo.h>

// --- Ultrasonic Pins ---
#define TRIG_PIN   5
#define ECHO_PIN   4

// --- Servo Pin ---
#define SERVO_PIN  13

// --- Scan configuration ---
const int SCAN_ANGLES[]   = {0, 30, 60, 90, 120, 150, 180};
const int NUM_ANGLES      = sizeof(SCAN_ANGLES) / sizeof(SCAN_ANGLES[0]);
#define SETTLE_TIME_MS     300   // time for servo to settle before reading
#define MAX_RANGE_CM       200.0f

// ============================================================

Servo myServo;

struct ScanResult {
  int   angle;
  float distanceCm;
};

ScanResult scanData[7];

// ============================================================

float readDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1.0f;
  return (duration * 0.0343f) / 2.0f;
}

// ============================================================

void runScan()
{
  Serial.println();
  Serial.println("============================");
  Serial.println("  OBSTACLE SCAN STARTING");
  Serial.println("============================");

  for (int i = 0; i < NUM_ANGLES; i++)
  {
    int angle = SCAN_ANGLES[i];

    myServo.write(angle);
    delay(SETTLE_TIME_MS);  // wait for servo to reach position

    float dist = readDistance();

    scanData[i].angle      = angle;
    scanData[i].distanceCm = dist;

    Serial.print("  Angle: ");
    Serial.print(angle);
    Serial.print("°  →  ");

    if (dist < 0 || dist > MAX_RANGE_CM)
      Serial.println("CLEAR (no obstacle)");
    else
    {
      Serial.print(dist, 1);
      Serial.println(" cm");
    }
  }

  // Return servo to center
  myServo.write(90);

  // Print summary table
  Serial.println();
  Serial.println("--- SCAN SUMMARY ---");
  Serial.println("  Angle  |  Distance");
  Serial.println("  -------+-----------");
  for (int i = 0; i < NUM_ANGLES; i++)
  {
    Serial.print("   ");
    if (scanData[i].angle < 100) Serial.print(" ");
    if (scanData[i].angle < 10)  Serial.print(" ");
    Serial.print(scanData[i].angle);
    Serial.print("°   |  ");

    if (scanData[i].distanceCm < 0 || scanData[i].distanceCm > MAX_RANGE_CM)
      Serial.println("CLEAR");
    else
    {
      Serial.print(scanData[i].distanceCm, 1);
      Serial.println(" cm");
    }
  }
  Serial.println();
  Serial.println("Send 'S' to scan again.");
}

// ============================================================

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(90);  // center on startup

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  STAGE 6 — COMBINED SCAN");
  Serial.println("=========================================");
  Serial.println("  TRIG→GPIO5  ECHO→GPIO4  Servo→GPIO13");
  Serial.println();
  Serial.println("Send 'S' to start a scan.");
  Serial.println();

  delay(500);
}

// ============================================================

void loop()
{
  if (Serial.available())
  {
    char cmd = Serial.read();
    if (cmd == 'S' || cmd == 's')
      runScan();
  }
}
