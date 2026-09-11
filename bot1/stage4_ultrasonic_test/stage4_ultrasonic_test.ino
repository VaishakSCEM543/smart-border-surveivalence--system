// ============================================================
// STAGE 4 — HC-SR04 ULTRASONIC RANGING TEST
// Bot 1 | ESP32 + HC-SR04
// ============================================================
// PURPOSE:
//   Verify the HC-SR04 gives accurate distance readings.
//   No motors, no servo, no ESP-NOW. Just ranging.
//
// CONNECTIONS:
//   TRIG → GPIO 5
//   ECHO → GPIO 4  (via voltage divider: ECHO→1kΩ→GPIO4→2kΩ→GND)
//   VCC  → 5V (external or L298N 5V output)
//   GND  → Common GND
//
// ⚠ VOLTAGE DIVIDER IS REQUIRED:
//   HC-SR04 ECHO outputs 5V. ESP32 GPIO is 3.3V max.
//   Without the divider you risk damaging the ESP32.
//   Divider: ECHO pin → 1kΩ → GPIO4 → 2kΩ → GND
//
// HOW TO USE:
//   1. Wire HC-SR04 with voltage divider
//   2. Upload, open Serial Monitor at 115200
//   3. Hold objects at known distances (10cm, 20cm, 30cm)
//   4. Verify readings are accurate (±2 cm is acceptable)
//
// PASS CRITERIA:
//   ✓ Readings update continuously
//   ✓ Distance increases when you move object away
//   ✓ "OUT OF RANGE" shown when nothing within ~200cm
// ============================================================

#define TRIG_PIN   5
#define ECHO_PIN   4

// Speed of sound: 0.0343 cm/µs (at ~20°C)
// Distance = (pulse_duration_µs × 0.0343) / 2
#define SOUND_CM_PER_US  0.0343f

// Max reliable range for HC-SR04 ≈ 200 cm
#define MAX_RANGE_CM   200.0f

// ============================================================

float readDistance()
{
  // Trigger pulse: 10µs HIGH
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse width (timeout = 30ms → ~515 cm, safely beyond range)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return -1.0f;  // timeout / no echo

  float distance = (duration * SOUND_CM_PER_US) / 2.0f;
  return distance;
}

// ============================================================

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  STAGE 4 — ULTRASONIC RANGING TEST");
  Serial.println("=========================================");
  Serial.println("  TRIG → GPIO5   ECHO → GPIO4");
  Serial.println("  ⚠ Voltage divider required on ECHO!");
  Serial.println();
}

// ============================================================

void loop()
{
  float dist = readDistance();

  Serial.print("Distance: ");

  if (dist < 0 || dist > MAX_RANGE_CM)
    Serial.println("OUT OF RANGE");
  else
  {
    Serial.print(dist, 1);
    Serial.println(" cm");
  }

  delay(200);
}
