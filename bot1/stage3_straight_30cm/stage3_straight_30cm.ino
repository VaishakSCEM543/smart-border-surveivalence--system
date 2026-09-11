// ============================================================
// STAGE 3 — STRAIGHT 30 cm MOVEMENT (FINAL BULLETPROOF VERSION)
// ============================================================
// HOW TO USE:
//   1. Place robot on floor
//   2. Upload, open Serial Monitor at 115200
//   3. Send 'G' to start
// ============================================================

#define IN1  27
#define IN2  26
#define IN3  25
#define IN4  33
#define ENA  14
#define ENB  32

// CORRECTED ENCODER PINS
#define LEFT_A   21
#define LEFT_B   22
#define RIGHT_A  18
#define RIGHT_B  19

// CALIBRATION
#define COUNTS_PER_CM   66.7f
#define TARGET_CM       30.0f
#define TARGET_COUNTS   2001

// BULLETPROOF SPEED SETTINGS for USB Power
#define BASE_SPEED    120     // High enough to guarantee movement
#define MIN_SPEED     85      // NEVER DROP BELOW THIS (prevents USB power crash)
#define MAX_SPEED     255
#define KP            0.5f    // Gentle correction, no violent swings

volatile long leftCount  = 0;
volatile long rightCount = 0;

void IRAM_ATTR leftISR() {
  if (digitalRead(LEFT_A) == digitalRead(LEFT_B)) leftCount--;
  else leftCount++;
}

void IRAM_ATTR rightISR() {
  if (digitalRead(RIGHT_A) == digitalRead(RIGHT_B)) rightCount++;
  else rightCount--;
}

void leftMotor(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0)      { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); }
  else if (speed < 0) { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  }
  else                { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  }
  analogWrite(ENA, abs(speed));
}

void rightMotor(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0)      { digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
  else if (speed < 0) { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  }
  else                { digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  }
  analogWrite(ENB, abs(speed));
}

void stopMotors() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

bool running = false;
unsigned long lastControlTime = 0;

void resetAndRun() {
  noInterrupts();
  leftCount  = 0;
  rightCount = 0;
  interrupts();
  running = true;
  lastControlTime = millis();
  Serial.println("\n>>> Starting 30 cm run...");
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(LEFT_A,  INPUT_PULLUP); pinMode(LEFT_B,  INPUT_PULLUP);
  pinMode(RIGHT_A, INPUT_PULLUP); pinMode(RIGHT_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_A),  leftISR,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_A), rightISR, CHANGE);
  stopMotors();

  Serial.println("\n=============================================");
  Serial.println("  STAGE 3 FINAL — ANTI-STALL SYNC");
  Serial.println("=============================================");
  Serial.println("Send 'G' to go. Place robot on floor first.");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if ((cmd == 'G' || cmd == 'g') && !running) resetAndRun();
  }

  if (!running) return;

  if (millis() - lastControlTime >= 20) {
    lastControlTime = millis();

    noInterrupts();
    long L_raw = leftCount;
    long R_raw = rightCount;
    interrupts();

    // Absolute values ignore all wiring/polarity issues
    long L = abs(L_raw);
    long R = abs(R_raw);
    long avg = (L + R) / 2;

    if (avg >= TARGET_COUNTS) {
      stopMotors();
      running = false;
      Serial.println("\n>>> STOPPED. Target reached.");
      Serial.print("Est Distance: "); 
      Serial.print((float)avg / COUNTS_PER_CM, 1); 
      Serial.println(" cm");
      return;
    }

    // Smooth proportional control
    long error = L - R;
    int correction = (int)(KP * error);

    int leftSpd  = BASE_SPEED - correction;
    int rightSpd = BASE_SPEED + correction;

    // THE MAGIC FIX: NEVER let speed drop below 85 to prevent USB brownout
    leftSpd  = constrain(leftSpd,  MIN_SPEED, MAX_SPEED);
    rightSpd = constrain(rightSpd, MIN_SPEED, MAX_SPEED);

    leftMotor(leftSpd);
    rightMotor(rightSpd);
    
    // Print progress occasionally to avoid serial lag
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 300) {
        lastPrint = millis();
        Serial.print("L="); Serial.print(L);
        Serial.print("  R="); Serial.print(R);
        Serial.print("  spd L="); Serial.print(leftSpd);
        Serial.print("  R="); Serial.println(rightSpd);
    }
  }
}
