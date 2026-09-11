// ============================================================
// STAGE 4 — ULTRASONIC RANGING TEST (HC-SR04)
// Bot 1 | ESP32
// ============================================================
// HOW TO USE:
//   1. Ensure HC-SR04 is wired:
//        VCC  → 5V (VIN if powered by USB)
//        GND  → GND
//        TRIG → GPIO 5
//        ECHO → GPIO 4 (MUST use 1k/2k voltage divider to protect ESP32!)
//   2. Upload and open Serial Monitor at 115200 baud.
//   3. Put your hand in front of the sensor to test distances.
// ============================================================

#define TRIG_PIN  5
#define ECHO_PIN  4

// Speed of sound = 343 m/s = 0.0343 cm/us
const float SOUND_SPEED = 0.0343f;

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  digitalWrite(TRIG_PIN, LOW);

  Serial.println("\n=============================================");
  Serial.println("  STAGE 4 — ULTRASONIC TEST");
  Serial.println("=============================================");
}

void loop() {
  // 1. Send 10 microsecond pulse to TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 2. Measure ECHO pulse duration (timeout after 25ms, ~400cm max range)
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);

  if (duration == 0) {
    Serial.println("Obstacle > 400cm or Sensor Disconnected");
  } else {
    // 3. Calculate distance (divide by 2 for round trip)
    float distanceCm = (duration * SOUND_SPEED) / 2.0;
    
    Serial.print("Distance: ");
    Serial.print(distanceCm, 1);
    Serial.println(" cm");
  }

  // Read 4 times a second
  delay(250);
}
