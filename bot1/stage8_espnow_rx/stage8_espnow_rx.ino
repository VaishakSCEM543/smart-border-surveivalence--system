// ============================================================
// STAGE 8B — ESP-NOW RECEIVER (Bot 2)
// Bot 2 | ESP32
// ============================================================
// PURPOSE:
//   Bot 2 listens for the obstacle data packet from Bot 1
//   and prints it on Serial Monitor. Minimum viable demo receiver.
//
// GIVE THIS FILE TO YOUR FRIEND.
//
// CONNECTIONS:
//   Just USB to ESP32. Nothing else needed for the receiver demo.
//
// SETUP:
//   1. Upload this to Bot 2's ESP32
//   2. Open Serial Monitor at 115200 baud
//   3. Note the MAC address printed at startup
//   4. Send that MAC address to your partner (Bot 1 user)
//   5. Bot 1 user puts it in stage8_espnow_tx.ino → BOT2_MAC
//
// PASS CRITERIA:
//   ✓ When Bot 1 runs, this Serial Monitor shows the received packet
// ============================================================

#include <esp_now.h>
#include <WiFi.h>

// ============================================================
// Data packet structure — MUST match Bot 1's definition exactly
// ============================================================
typedef struct ObstaclePacket {
  uint8_t  botID;
  float    distanceTraveled;
  int      angles[7];
  float    distances[7];
} ObstaclePacket;

// ============================================================

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len)
{
  if (len != sizeof(ObstaclePacket))
  {
    Serial.println("Received packet with unexpected size — ignoring.");
    return;
  }

  ObstaclePacket pkt;
  memcpy(&pkt, data, sizeof(pkt));

  Serial.println();
  Serial.println("========================================");
  Serial.println("  PACKET RECEIVED FROM BOT 1  ✓");
  Serial.println("========================================");
  Serial.print  ("  BotID      : "); Serial.println(pkt.botID);
  Serial.print  ("  Distance   : "); Serial.print(pkt.distanceTraveled, 1); Serial.println(" cm");
  Serial.println("  Obstacle Scan:");

  for (int i = 0; i < 7; i++)
  {
    Serial.print("    ");
    if (pkt.angles[i] < 100) Serial.print(" ");
    if (pkt.angles[i] < 10)  Serial.print(" ");
    Serial.print(pkt.angles[i]);
    Serial.print("° : ");

    if (pkt.distances[i] < 0)
      Serial.println("CLEAR");
    else
    {
      Serial.print(pkt.distances[i], 1);
      Serial.println(" cm");
    }
  }

  Serial.println("========================================");
  Serial.println();
}

// ============================================================

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW init FAILED.");
    while (true) delay(1000);
  }

  esp_now_register_recv_cb(onDataReceived);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  STAGE 8 — BOT 2 — ESP-NOW RECEIVER");
  Serial.println("=========================================");
  Serial.print  ("  My MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
  Serial.println("  ★ SEND THIS MAC ADDRESS TO YOUR PARTNER (Bot 1) ★");
  Serial.println("  They paste it into stage8_espnow_tx.ino → BOT2_MAC");
  Serial.println();
  Serial.println("  Waiting for packets from Bot 1...");
  Serial.println();
}

// ============================================================

void loop()
{
  // Nothing to do — ESP-NOW callback handles everything
  delay(100);
}
