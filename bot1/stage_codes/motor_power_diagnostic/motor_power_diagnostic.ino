// ============================================================
// MOTOR POWER DIAGNOSTIC TEST
// Bot 1 | ESP32 + L298N + Encoders
// ============================================================
// PURPOSE:
//   Test each motor independently AND together to determine:
//   1. Whether each motor can spin at a given PWM
//   2. The encoder counts/second (= actual speed)
//   3. Whether running BOTH motors causes power starvation
//
// HOW TO READ RESULTS:
//   counts/sec × 60 ÷ 712 = RPM  (712 = counts/revolution)
//   Expected at PWM 100 on 5V: roughly 30-80 RPM
//   If counts/sec < 10 → motor is stalling (not enough power)
//   If LEFT >> RIGHT at same PWM → imbalance confirmed
//   If individual OK but BOTH low → power supply is the bottleneck
//
// CONNECTIONS: Same as Stage 2 (encoders + motors)
// SEND COMMANDS via Serial Monitor:
//   '1' → Test LEFT motor only
//   '2' → Test RIGHT motor only
//   '3' → Test BOTH motors simultaneously
//   'A' → Auto-run all 3 tests in sequence
// ============================================================

// --- Motor Pins ---
#define IN1  27
#define IN2  26
#define IN3  25
#define IN4  33
#define ENA  14
#define ENB  32

// --- Encoder Pins --- CORRECTED: physically swapped
// LEFT motor encoder  → physically on GPIO21/22
// RIGHT motor encoder → physically on GPIO18/19
#define LEFT_A   21
#define LEFT_B   22
#define RIGHT_A  18
#define RIGHT_B  19

// --- Test settings ---
#define TEST_PWM      100     // PWM value to test at (0-255)
#define TEST_SECS     3       // run duration per test
#define COUNTS_PER_REV 712.0f // from Stage 2

// ============================================================

volatile long leftCount  = 0;
volatile long rightCount = 0;

void IRAM_ATTR leftISR()
{
  // Left motor encoder on GPIO21/22: forward = negative → flip it
  if (digitalRead(LEFT_A) == digitalRead(LEFT_B)) leftCount--;
  else leftCount++;
}

void IRAM_ATTR rightISR()
{
  // Right motor encoder on GPIO18/19: forward = positive → keep
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

void resetCounts()
{
  noInterrupts();
  leftCount  = 0;
  rightCount = 0;
  interrupts();
}

void printResult(const char* label, long L, long R, int secs)
{
  float Lps  = abs(L) / (float)secs;   // counts per second
  float Rps  = abs(R) / (float)secs;
  float Lrpm = Lps * 60.0f / COUNTS_PER_REV;
  float Rrpm = Rps * 60.0f / COUNTS_PER_REV;

  Serial.println();
  Serial.print("=== "); Serial.print(label); Serial.println(" ===");
  Serial.print("  Left  : "); Serial.print(L);
  Serial.print(" counts  ("); Serial.print(Lps, 1);
  Serial.print(" c/s  ~"); Serial.print(Lrpm, 1); Serial.println(" RPM)");
  Serial.print("  Right : "); Serial.print(R);
  Serial.print(" counts  ("); Serial.print(Rps, 1);
  Serial.print(" c/s  ~"); Serial.print(Rrpm, 1); Serial.println(" RPM)");

  // Power assessment
  if (L == 0 && R == 0)
    Serial.println("  ⚠ BOTH STALLED — no power reaching motors");
  else if (abs(L) < 20)
    Serial.println("  ⚠ LEFT  STALLED — check wiring or power");
  else if (abs(R) < 20)
    Serial.println("  ⚠ RIGHT STALLED — check wiring or power");
  else if (Lrpm > 0 && Rrpm > 0)
  {
    float ratio = (Lrpm > Rrpm) ? Lrpm / Rrpm : Rrpm / Lrpm;
    if (ratio > 2.0f)
      Serial.println("  ⚠ LARGE IMBALANCE — one motor much faster than other");
    else if (ratio > 1.3f)
      Serial.println("  ~ Minor imbalance — PI sync should handle this");
    else
      Serial.println("  ✓ Both motors well-balanced");
  }
  Serial.println();
}

// ============================================================

void runTest(const char* label, bool runLeft, bool runRight)
{
  Serial.println();
  Serial.print(">>> Starting: "); Serial.println(label);
  Serial.print("    PWM = "); Serial.print(TEST_PWM);
  Serial.print("  Duration = "); Serial.print(TEST_SECS); Serial.println("s");
  Serial.println("    Running...");

  resetCounts();

  unsigned long startMs = millis();
  unsigned long endMs   = startMs + (TEST_SECS * 1000UL);

  // Print live counts every 500ms during test
  unsigned long lastPrint = 0;

  while (millis() < endMs)
  {
    if (runLeft)  leftMotor(TEST_PWM);
    if (runRight) rightMotor(TEST_PWM);

    if (millis() - lastPrint >= 500)
    {
      lastPrint = millis();
      noInterrupts();
      long L = leftCount;
      long R = rightCount;
      interrupts();
      Serial.print("    L="); Serial.print(L);
      Serial.print("  R="); Serial.println(R);
    }
  }

  stopMotors();

  noInterrupts();
  long finalL = leftCount;
  long finalR = rightCount;
  interrupts();

  printResult(label, finalL, finalR, TEST_SECS);
  delay(1500);  // rest between tests
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

  stopMotors();

  Serial.println();
  Serial.println("=============================================");
  Serial.println("  MOTOR POWER DIAGNOSTIC TEST");
  Serial.println("=============================================");
  Serial.print  ("  TEST_PWM      = "); Serial.println(TEST_PWM);
  Serial.print  ("  TEST_DURATION = "); Serial.print(TEST_SECS); Serial.println(" sec each");
  Serial.print  ("  COUNTS/REV    = "); Serial.println(COUNTS_PER_REV);
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  '1' → Test LEFT  motor only");
  Serial.println("  '2' → Test RIGHT motor only");
  Serial.println("  '3' → Test BOTH  motors together");
  Serial.println("  'A' → Run ALL 3 tests in sequence");
  Serial.println();
  Serial.println("Lift robot off ground before testing!");
  Serial.println();
}

// ============================================================

void loop()
{
  if (Serial.available())
  {
    char cmd = Serial.read();

  if (cmd == '1')
      runTest("LEFT ONLY  (PWM=100)", true,  false);
    else if (cmd == '2')
      runTest("RIGHT ONLY (PWM=100)", false, true);
    else if (cmd == '3')
      runTest("BOTH MOTORS (PWM=100)", true,  true);
    else if (cmd == 'A' || cmd == 'a')
    {
      Serial.println(">>> AUTO TEST --- running all 3 in sequence...");
      delay(2000);
      runTest("LEFT ONLY  (PWM=100)", true,  false);
      runTest("RIGHT ONLY (PWM=100)", false, true);
      runTest("BOTH MOTORS (PWM=100)", true,  true);
      Serial.println(">>> ALL TESTS COMPLETE");
      Serial.println("Send 'A' to repeat.");
    }
  }
}


