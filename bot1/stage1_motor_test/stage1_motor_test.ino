// ============================================================
// STAGE 1 — MOTOR TEST
// Bot 1 | ESP32 + L298N
// ============================================================
// PURPOSE:
//   Verify all 4 movement directions are physically correct.
//   Run FORWARD → STOP → REVERSE → STOP → LEFT → STOP → RIGHT → STOP
//   Watch Serial Monitor to confirm what you see physically.
//
// CONNECTIONS USED IN THIS STAGE:
//   IN1 → GPIO27   IN2 → GPIO26   (Left motor direction)
//   IN3 → GPIO25   IN4 → GPIO33   (Right motor direction)
//   ENA → GPIO14   ENB → GPIO32   (PWM speed)
//
// BEFORE UPLOADING:
//   1. Remove ENA/ENB jumpers from L298N
//   2. Lift robot off the ground (wheels free to spin)
//   3. Connect battery to L298N
//   4. Open Serial Monitor at 115200 baud
//
// PASS CRITERIA:
//   ✓ FORWARD  → both wheels spin forward
//   ✓ REVERSE  → both wheels spin backward
//   ✓ LEFT     → robot rotates left
//   ✓ RIGHT    → robot rotates right
// ============================================================

// --- Motor Driver Pins ---
#define IN1  27
#define IN2  26
#define IN3  25
#define IN4  33
#define ENA  14
#define ENB  32

// --- PWM speed for this test (0-255) ---
// Lowered to 100 because running from USB 5V (no battery)
#define TEST_SPEED  100

// ============================================================

void leftMotor(int speed)
{
  speed = constrain(speed, -255, 255);

  if (speed > 0)       { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); }
  else if (speed < 0)  { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  }
  else                 { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  }

  analogWrite(ENA, abs(speed));
}

void rightMotor(int speed)
{
  speed = constrain(speed, -255, 255);

  if (speed > 0)       { digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
  else if (speed < 0)  { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  }
  else                 { digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  }

  analogWrite(ENB, abs(speed));
}

void stopMotors()
{
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

// ============================================================

void setup()
{
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  stopMotors();

  Serial.println();
  Serial.println("=====================================");
  Serial.println("  STAGE 1 — MOTOR TEST");
  Serial.println("=====================================");
  Serial.println("Starting in 3 seconds...");
  Serial.println("Make sure robot is OFF the ground!");
  delay(3000);
}

// ============================================================

void runTest(const char* label, int leftSpd, int rightSpd, int durationMs)
{
  Serial.print(">>> ");
  Serial.println(label);
  leftMotor(leftSpd);
  rightMotor(rightSpd);
  delay(durationMs);
  stopMotors();
  Serial.println("    STOP");
  delay(1500);
}

void loop()
{
  runTest("FORWARD",  TEST_SPEED,  TEST_SPEED,  2000);
  runTest("REVERSE", -TEST_SPEED, -TEST_SPEED,  2000);
  runTest("LEFT",     TEST_SPEED, -TEST_SPEED,  1500);
  runTest("RIGHT",   -TEST_SPEED,  TEST_SPEED,  1500);

  Serial.println();
  Serial.println("--- Cycle complete. Repeating in 3s ---");
  Serial.println();
  delay(3000);
}
