// ============================================================
// HELPER — GET MAC ADDRESS
// Upload to any ESP32 to read its MAC address
// ============================================================
// USE CASE:
//   Bot 2 must tell Bot 1 its MAC address so ESP-NOW can pair.
//   Upload this to Bot 2, open Serial Monitor, copy the MAC.
//
// Your friend should:
//   1. Upload this sketch to Bot 2
//   2. Open Serial Monitor at 115200
//   3. Read the line: "MAC Address: XX:XX:XX:XX:XX:XX"
//   4. Send that MAC to you (Bot 1 user)
// ============================================================

#include <WiFi.h>

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  MAC ADDRESS HELPER");
  Serial.println("=========================================");
  Serial.print  ("  MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
  Serial.println("  Copy the MAC address above and send it");
  Serial.println("  to your partner (Bot 1 user).");
  Serial.println("=========================================");
}

void loop()
{
  // Print every 5 seconds so it's easy to read
  delay(5000);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
}
