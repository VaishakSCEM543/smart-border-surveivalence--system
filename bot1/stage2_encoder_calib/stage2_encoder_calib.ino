// ============================================================
// STAGE 2 — ENCODER CALIBRATION
// Bot 1 | ESP32 + Both N20 Encoder Motors
// ============================================================
// PURPOSE:
//   Find the exact encoder counts per one full wheel revolution
//   for BOTH the left and right wheels independently.
//   This number is the foundation of all distance control.
//
// CONNECTIONS USED IN THIS STAGE:
//   Left  encoder A → GPIO18  (INPUT_PULLUP)
//   Left  encoder B → GPIO19  (INPUT_PULLUP)
//   Right encoder A → GPIO21  (INPUT_PULLUP)
//   Right encoder B → GPIO22  (INPUT_PULLUP)
//   Motors NOT needed for this test (hand rotation only)
//
// HOW TO USE:
//   1. Upload this sketch
//   2. Open Serial Monitor at 115200 baud
//   3. Send 'L' to reset left encoder → rotate left wheel EXACTLY 1 revolution
//   4. Note the left count shown
//   5. Send 'R' to reset right encoder → rotate right wheel EXACTLY 1 revolution
//   6. Note the right count shown
//   7. Repeat 3x each, average the results
//   8. Write results in PROJECT_TRACKER.txt
//
// PASS CRITERIA:
//   ✓ Counts increase when wheel rotates forward
//   ✓ Counts decrease when wheel rotates backward
//   ✓ Both wheels give consistent count per revolution
// ============================================================

// --- Encoder Pins ---
#define LEFT_A   18
#define LEFT_B   19
#define RIGHT_A  21
#define RIGHT_B  22

// --- Encoder counters ---
volatile long leftCount  = 0;
volatile long rightCount = 0;

// ============================================================

void IRAM_ATTR leftISR()
{
  if (digitalRead(LEFT_A) == digitalRead(LEFT_B))
    leftCount++;
  else
    leftCount--;
}

void IRAM_ATTR rightISR()
{
  if (digitalRead(RIGHT_A) == digitalRead(RIGHT_B))
    rightCount++;
  else
    rightCount--;
}

// ============================================================

void setup()
{
  Serial.begin(115200);

  pinMode(LEFT_A,  INPUT_PULLUP);
  pinMode(LEFT_B,  INPUT_PULLUP);
  pinMode(RIGHT_A, INPUT_PULLUP);
  pinMode(RIGHT_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_A),  leftISR,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_A), rightISR, CHANGE);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  STAGE 2 — ENCODER CALIBRATION");
  Serial.println("=========================================");
  Serial.println("Commands:");
  Serial.println("  L → Reset LEFT  counter, then rotate left  wheel 1 full revolution");
  Serial.println("  R → Reset RIGHT counter, then rotate right wheel 1 full revolution");
  Serial.println("  B → Reset BOTH counters");
  Serial.println();
  Serial.println("Rotate wheels slowly and steadily by hand.");
  Serial.println();
}

// ============================================================

void loop()
{
  // Handle serial commands
  if (Serial.available())
  {
    char cmd = Serial.read();

    if (cmd == 'L' || cmd == 'l')
    {
      noInterrupts();
      leftCount = 0;
      interrupts();
      Serial.println(">>> LEFT reset. Rotate left wheel ONE full revolution now.");
    }
    else if (cmd == 'R' || cmd == 'r')
    {
      noInterrupts();
      rightCount = 0;
      interrupts();
      Serial.println(">>> RIGHT reset. Rotate right wheel ONE full revolution now.");
    }
    else if (cmd == 'B' || cmd == 'b')
    {
      noInterrupts();
      leftCount  = 0;
      rightCount = 0;
      interrupts();
      Serial.println(">>> BOTH encoders reset.");
    }
  }

  // Print counts every 300ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 300)
  {
    lastPrint = millis();

    noInterrupts();
    long L = leftCount;
    long R = rightCount;
    interrupts();

    Serial.print("LEFT = ");
    Serial.print(L);
    Serial.print("    RIGHT = ");
    Serial.println(R);
  }
}
