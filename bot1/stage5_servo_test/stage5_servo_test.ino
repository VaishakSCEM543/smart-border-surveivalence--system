// ============================================================
// STAGE 5 — SERVO SWEEP TEST
// Bot 1 | ESP32 + SG90 Servo
// ============================================================
// PURPOSE:
//   Verify the SG90 servo moves to correct positions.
//   Test positions: 0° → 45° → 90° → 135° → 180° → back
//
// CONNECTIONS:
//   Signal → GPIO 13
//   VCC    → External 5V (NOT ESP32 3.3V — servo draws too much current)
//   GND    → Common GND
//
// ⚠ Do NOT power servo from ESP32 3.3V.
//    It will cause voltage drops and ESP32 resets.
//
// LIBRARY REQUIRED:
//   Arduino IDE → Sketch → Include Library → Manage Libraries
//   Search: "ESP32Servo"  →  Install "ESP32Servo"
//
// HOW TO USE:
//   1. Connect servo with external 5V
//   2. Upload and open Serial Monitor at 115200
//   3. Send angle (0-180) via Serial to move to that position
//   4. Servo auto-sweeps if no input
//
// PASS CRITERIA:
//   ✓ Servo reaches all positions without jitter
//   ✓ 0° = one extreme, 90° = center, 180° = other extreme
//   ✓ No ESP32 resets during servo movement
// ============================================================

#include <ESP32Servo.h>

#define SERVO_PIN  13

Servo myServo;

// Scan positions (degrees)
const int SCAN_ANGLES[] = {0, 30, 60, 90, 120, 150, 180};
const int NUM_ANGLES = sizeof(SCAN_ANGLES) / sizeof(SCAN_ANGLES[0]);

// ============================================================

void setup()
{
  Serial.begin(115200);

  // Attach servo — ESP32Servo uses ledc PWM
  myServo.attach(SERVO_PIN, 500, 2400);  // min/max pulse width µs

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  STAGE 5 — SERVO SWEEP TEST");
  Serial.println("=========================================");
  Serial.println("  Signal → GPIO13   VCC → external 5V");
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  Send a number (0-180) to move servo to that angle");
  Serial.println("  Send 'S' to start auto-sweep");
  Serial.println();

  // Start at center
  myServo.write(90);
  delay(500);
  Serial.println("Servo at 90° (center). Ready.");
  Serial.println();
}

// ============================================================

bool autoSweep = true;
int sweepIndex = 0;

void loop()
{
  // Check for serial input
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "S" || input == "s")
    {
      autoSweep = true;
      sweepIndex = 0;
      Serial.println(">>> Auto-sweep started");
    }
    else
    {
      int angle = input.toInt();
      if (angle >= 0 && angle <= 180)
      {
        autoSweep = false;
        myServo.write(angle);
        Serial.print(">>> Moved to ");
        Serial.print(angle);
        Serial.println("°");
      }
      else
      {
        Serial.println("Invalid. Send 0-180 or 'S' for sweep.");
      }
    }
  }

  // Auto-sweep through scan positions
  if (autoSweep)
  {
    static unsigned long lastMove = 0;

    if (millis() - lastMove >= 700)
    {
      lastMove = millis();

      int angle = SCAN_ANGLES[sweepIndex];
      myServo.write(angle);

      Serial.print("Servo → ");
      Serial.print(angle);
      Serial.println("°");

      sweepIndex++;
      if (sweepIndex >= NUM_ANGLES)
      {
        sweepIndex = 0;
        Serial.println("--- Sweep complete, repeating ---");
      }
    }
  }
}
