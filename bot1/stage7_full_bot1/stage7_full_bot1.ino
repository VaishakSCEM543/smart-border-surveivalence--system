// ============================================================
// STAGE 7 — FULL BOT 1 (Move 30cm + Scan + Data Packet)
// Bot 1 | ESP32 + L298N + Encoders + SG90 + HC-SR04
// ============================================================
// PURPOSE:
//   Complete Bot 1 behavior:
//     1. Wait for 'G' command
//     2. Drive forward 30 cm (encoder-controlled, synchronized)
//     3. Stop
//     4. Run obstacle scan (7 angles)
//     5. Print complete data packet to Serial Monitor
//     6. Ready for Stage 8 (ESP-NOW transmission)
//
// ALL CONNECTIONS:
//   IN1→GPIO27  IN2→GPIO26  IN3→GPIO25  IN4→GPIO33
//   ENA→GPIO14  ENB→GPIO32
//   LEFT_A→GPIO18  LEFT_B→GPIO19
//   RIGHT_A→GPIO21 RIGHT_B→GPIO22
//   TRIG→GPIO5  ECHO→GPIO4 (voltage divider!)
//   Servo→GPIO13 (external 5V)
//
// UPDATE COUNTS_PER_CM with your Stage 2 result before using!
// ============================================================

#include <ESP32Servo.h>

// --- Motor Pins ---
#define IN1  27
#define IN2  26
#define IN3  25
#define IN4  33
#define ENA  14
#define ENB  32

// --- Encoder Pins --- CORRECTED: physically swapped
#define LEFT_A   21
#define LEFT_B   22
#define RIGHT_A  18
#define RIGHT_B  19

// --- Sensor Pins ---
#define TRIG_PIN   5
#define ECHO_PIN   4
#define SERVO_PIN  13

// ============================================================
// ★ CONFIRMED CALIBRATION FROM STAGE 2 ★
#define COUNTS_PER_CM   66.7f   // 712 counts/rev ÷ 10.68cm
#define TARGET_CM       30.0f
#define TARGET_COUNTS   (long)(TARGET_CM * COUNTS_PER_CM)  // = 2001

#define DRIVE_SPEED     130
#define SYNC_GAIN       0.4f
// ============================================================

// Scan config
const int SCAN_ANGLES[]  = {0, 30, 60, 90, 120, 150, 180};
const int NUM_ANGLES     = sizeof(SCAN_ANGLES) / sizeof(SCAN_ANGLES[0]);
#define SETTLE_TIME_MS   300
#define MAX_RANGE_CM     200.0f

// ============================================================

volatile long leftCount  = 0;
volatile long rightCount = 0;

Servo myServo;

// Scan data
float scanDistances[7];

// ============================================================

void IRAM_ATTR leftISR()
{
  if (digitalRead(LEFT_A) == digitalRead(LEFT_B)) leftCount--;
  else leftCount++;
}

void IRAM_ATTR rightISR()
{
  if (digitalRead(RIGHT_A) == digitalRead(RIGHT_B)) rightCount++;
  else rightCount--;
}

// ============================================================

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

// ============================================================

float readDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  if (dur == 0) return -1.0f;
  return (dur * 0.0343f) / 2.0f;
}

// ============================================================

void driveStraight()
{
  Serial.println(">>> PHASE 1: Driving 30 cm...");

  noInterrupts();
  leftCount = 0; rightCount = 0;
  interrupts();

  while (true)
  {
    noInterrupts();
    long L = leftCount;
    long R = rightCount;
    interrupts();

    long avg = (L + R) / 2;
    if (avg >= TARGET_COUNTS) break;

    long error = L - R;
    int lspd = constrain(DRIVE_SPEED - (int)(SYNC_GAIN * error), 60, 255);
    int rspd = constrain(DRIVE_SPEED + (int)(SYNC_GAIN * error), 60, 255);

    leftMotor(lspd);
    rightMotor(rspd);
  }

  stopMotors();

  noInterrupts();
  long finalL = leftCount;
  long finalR = rightCount;
  interrupts();

  float dist = ((float)(finalL + finalR) / 2.0f) / COUNTS_PER_CM;
  Serial.print(">>> STOPPED at ~");
  Serial.print(dist, 1);
  Serial.println(" cm");
  delay(500);
}

// ============================================================

void runScan()
{
  Serial.println(">>> PHASE 2: Scanning obstacles...");

  for (int i = 0; i < NUM_ANGLES; i++)
  {
    myServo.write(SCAN_ANGLES[i]);
    delay(SETTLE_TIME_MS);
    scanDistances[i] = readDistance();

    Serial.print("  ");
    Serial.print(SCAN_ANGLES[i]);
    Serial.print("° → ");
    if (scanDistances[i] < 0 || scanDistances[i] > MAX_RANGE_CM)
      Serial.println("CLEAR");
    else
    {
      Serial.print(scanDistances[i], 1);
      Serial.println(" cm");
    }
  }

  myServo.write(90);  // return to center
  delay(300);
}

// ============================================================

void printDataPacket()
{
  Serial.println();
  Serial.println("========================================");
  Serial.println("  BOT 1 DATA PACKET");
  Serial.println("========================================");
  Serial.println("  BotID      : 1");
  Serial.print  ("  Distance   : ");
  Serial.print  (TARGET_CM, 1);
  Serial.println(" cm");
  Serial.println("  Scan:");

  for (int i = 0; i < NUM_ANGLES; i++)
  {
    Serial.print("    ");
    Serial.print(SCAN_ANGLES[i]);
    Serial.print("° : ");
    if (scanDistances[i] < 0 || scanDistances[i] > MAX_RANGE_CM)
      Serial.println("CLEAR");
    else
    {
      Serial.print(scanDistances[i], 1);
      Serial.println(" cm");
    }
  }
  Serial.println("========================================");
  Serial.println();
  Serial.println("Send 'G' to run again.");
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

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  STAGE 7 — FULL BOT 1");
  Serial.println("  Move 30cm  →  Scan  →  Data Packet");
  Serial.println("=========================================");
  Serial.print  ("  COUNTS_PER_CM = "); Serial.println(COUNTS_PER_CM);
  Serial.print  ("  TARGET_COUNTS = "); Serial.println(TARGET_COUNTS);
  Serial.println();
  Serial.println("Place robot on floor. Send 'G' to go.");
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
      printDataPacket();
    }
  }
}
