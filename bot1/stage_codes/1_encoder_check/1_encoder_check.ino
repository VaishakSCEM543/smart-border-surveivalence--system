#define ENC_A 18
#define ENC_B 19

volatile long encoderCount = 0;

void IRAM_ATTR encoderISR()
{
  int A = digitalRead(ENC_A);
  int B = digitalRead(ENC_B);

  if (A == B)
    encoderCount++;
  else
    encoderCount--;
}

void setup()
{
  Serial.begin(115200);

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(ENC_A),
    encoderISR,
    CHANGE
  );

  Serial.println();
  Serial.println("================================");
  Serial.println("   ENCODER CALIBRATION TEST");
  Serial.println("================================");
  Serial.println();
  Serial.println("Send R to reset the count.");
  Serial.println("Then rotate wheel exactly ONE revolution.");
  Serial.println();
}

void loop()
{
  // Check Serial input
  if (Serial.available())
  {
    char command = Serial.read();

    if (command == 'R' || command == 'r')
    {
      noInterrupts();
      encoderCount = 0;
      interrupts();

      Serial.println();
      Serial.println("COUNT RESET!");
      Serial.println("Now rotate the wheel ONE complete revolution.");
      Serial.println();
    }
  }

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 500)
  {
    lastPrint = millis();

    noInterrupts();
    long count = encoderCount;
    interrupts();

    Serial.print("Encoder Count: ");
    Serial.println(count);
  }
}