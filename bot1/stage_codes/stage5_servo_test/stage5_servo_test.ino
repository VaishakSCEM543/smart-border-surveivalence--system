// ============================================================
// STAGE 5 — SERVO SWEEP TEST (SG90)
// Bot 1 | ESP32
// ============================================================
// ⚠️ PREREQUISITE:
//   You MUST install the "ESP32Servo" library in Arduino IDE:
//   Go to Sketch -> Include Library -> Manage Libraries
//   Search for "ESP32Servo" by Kevin Harrington and install it.
//
// HOW TO USE:
//   1. Ensure SG90 Servo is wired:
//        Orange/Yellow (Signal) → GPIO 13
//        Red (VCC)              → External 5V (or VIN) - NOT 3.3V!
//        Brown/Black (GND)      → GND
//   2. Upload and open Serial Monitor at 115200.
//   3. The servo should sweep left, center, right, center continuously.
// ============================================================

#include <ESP32Servo.h>

#define SERVO_PIN 12

Servo scanServo;

void setup() {
  Serial.begin(115200);
  
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  // Attach servo to pin with standard SG90 min/max pulse widths
  scanServo.setPeriodHertz(50); // Standard 50hz servo
  scanServo.attach(SERVO_PIN, 500, 2400); 

  Serial.println("\n=============================================");
  Serial.println("  STAGE 5 — SERVO SWEEP TEST");
  Serial.println("=============================================");
}

void loop() {
  Serial.println("Sweeping Left (180°)...");
  scanServo.write(180);
  delay(1000);

  Serial.println("Sweeping Center (90°)...");
  scanServo.write(90);
  delay(1000);

  Serial.println("Sweeping Right (0°)...");
  scanServo.write(0);
  delay(1000);

  Serial.println("Sweeping Center (90°)...");
  scanServo.write(90);
  delay(1000);
}
