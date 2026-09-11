# 🤖 ESP32 Swarm Bot 1 — Firmware

> **Presentation deadline: Tomorrow afternoon**
> Bot 1 travels 30 cm → scans obstacles → sends data to Bot 2 via ESP-NOW

---

## 📋 Progress Tracker

| Stage | File | Description | Status |
|-------|------|-------------|--------|
| 1 | `stage1_motor_test` | Verify all 4 movement directions | ✅ **DONE** |
| 2 | `stage2_encoder_calib` | Calibrate encoder counts/revolution | ✅ **DONE** |
| 3 | `stage3_straight_30cm` | Drive exactly 30 cm (encoder-controlled) | ✅ **DONE** |
| 4 | `stage4_ultrasonic_test` | Verify HC-SR04 distance readings | ✅ **DONE** |
| 5 | `stage5_servo_test` | Verify SG90 servo positions | ⬜ Next |
| 6 | `stage6_scan` | Servo sweep + obstacle distances | ✅ **SKIPPED (Merged)** |
| 7 | `stage7_full_bot1` | Full run: move + scan + data packet | ✅ **DONE** |
| 8 | `stage8_espnow_tx` | Transmit packet to Bot 2 via ESP-NOW | ⬜ |

---

## 🔌 Complete Pin Map

### Motor Driver (L298N)

```
ESP32              L298N
─────────────────────────
GPIO 27    ──►    IN1
GPIO 26    ──►    IN2      }  Left motor direction
GPIO 25    ──►    IN3
GPIO 33    ──►    IN4      }  Right motor direction
GPIO 14    ──►    ENA         Left motor PWM speed
GPIO 32    ──►    ENB         Right motor PWM speed
GND        ──►    GND
VIN (5V)   ──►    +12V/VS     Motor power (USB powered, no battery)
```

> ⚠️ **Remove ENA and ENB jumper caps from L298N before use**

### Motors → L298N

```
Left motor  M1  ──►  OUT1
Left motor  M2  ──►  OUT2

Right motor M1  ──►  OUT3
Right motor M2  ──►  OUT4
```

### Encoders (N20 6-pin: M1 | GND | C1 | C2 | VCC | M2)

```
ESP32              Left Encoder       Right Encoder
──────────────────────────────────────────────────
GPIO 18    ──►    C1
GPIO 19    ──►    C2
GPIO 21    ──►                        C1
GPIO 22    ──►                        C2
3.3V       ──►    VCC                 VCC
GND        ──►    GND                 GND
```

### HC-SR04 Ultrasonic

```
ESP32              HC-SR04
──────────────────────────────────────────────
GPIO 5     ──►    TRIG
GPIO 4     ──►    ECHO   ⚠️ via voltage divider!
5V                VCC
GND               GND
```

```
⚠️  ECHO VOLTAGE DIVIDER (required — HC-SR04 outputs 5V, ESP32 is 3.3V max)

HC-SR04 ECHO ──── 1kΩ ──── GPIO4
                              │
                            2kΩ
                              │
                            GND
```

### SG90 Servo

```
ESP32              SG90
──────────────────────────────────────────────
GPIO 13    ──►    Signal (orange/yellow wire)
External 5V ─►   VCC    (red wire)
GND         ─►   GND    (brown/black wire)
```

> ⚠️ **Do NOT power servo from ESP32 3.3V** — it will cause resets

---

## 🔋 Power Architecture

```
USB Cable
    │
    ▼
 ESP32
    │
    ├── VIN ──────────────► L298N +12V/VS (motor supply)
    │
    ├── 3.3V ─────────────► Encoder VCC (both motors)
    │
    └── GND ──┬───────────► L298N GND
              ├───────────► Encoder GND (both motors)
              └───────────► HC-SR04 GND / Servo GND
```

> ✅ Common ground between ESP32, L298N, encoders, sensors is **mandatory**

---

## ✅ Stage 1 Results — Motor Test

| Test | Physical Result | Status |
|------|----------------|--------|
| FORWARD | Both wheels spin forward | ✅ |
| REVERSE | Both wheels spin backward | ✅ |
| LEFT | Robot rotates left | ✅ |
| RIGHT | Robot rotates right | ✅ |

**Confirmed motor direction logic:**
```cpp
// Forward:   IN1=LOW,  IN2=HIGH  (left)   IN3=LOW,  IN4=HIGH  (right)
// Backward:  IN1=HIGH, IN2=LOW   (left)   IN3=HIGH, IN4=LOW   (right)
// Left turn: leftMotor(+),  rightMotor(-)
// Right turn:leftMotor(-),  rightMotor(+)
```

> 📝 Note: Uneven motor speed at same PWM is **normal** (manufacturing tolerance).
> This is corrected automatically by the encoder sync in Stage 3.

---

## 📐 Calibration Data

| Parameter | Value | Source |
|-----------|-------|--------|
| Wheel diameter | 34 mm | Measured |
| Wheel circumference | π × 34 = **106.8 mm** | Calculated |
| Left encoder counts/rev | **715** | ✅ Confirmed Stage 2 |
| Right encoder counts/rev | **709** | ✅ Confirmed Stage 2 |
| Average counts/rev | **712** | ✅ Confirmed Stage 2 |
| Counts per cm | 712 ÷ 10.68 = **66.7** | ✅ Confirmed Stage 2 |
| Target for 30 cm | 30 × 66.7 = **2001 counts** | ✅ Set in Stage 3 |

---

## 📁 File Structure

```
bot1/
├── README.md                         ← You are here
│
├── information/
│   ├── PROJECT_TRACKER.txt           ← Detailed tracker
│   ├── info1.txt                     ← ChatGPT chat reference 1
│   └── info2fromchatgptchat.txt      ← ChatGPT chat reference 2
│
├── stage1_motor_test/
│   └── stage1_motor_test.ino         ✅ DONE
│
├── stage2_encoder_calib/
│   └── stage2_encoder_calib.ino      ✅ DONE
│
├── stage3_straight_30cm/
│   └── stage3_straight_30cm.ino      ✅ DONE
│
├── stage4_ultrasonic_test/
│   └── stage4_ultrasonic_test.ino    ✅ DONE
│
├── stage5_servo_test/
│   └── stage5_servo_test.ino         ⬅ DO THIS NEXT
│
├── stage6_scan/
│   └── stage6_scan.ino               ✅ SKIPPED (Merged to 7)
│
├── stage7_full_bot1/
│   └── stage7_full_bot1.ino          ✅ DONE (Demo Target)
│
├── stage8_espnow_tx/
│   └── stage8_espnow_tx.ino          ← Bot 1 transmitter
│
├── stage8_espnow_rx/
│   └── stage8_espnow_rx.ino          ← Bot 2 (friend's bot)
│
└── helper_get_mac/
    └── helper_get_mac.ino            ← Friend runs this first
```

---

## 🎯 Demo Plan (Presentation)

```
[Bot 1]                              [Bot 2 — Friend]
   │                                       │
   │  1. Power on                          │  1. Power on
   │  2. Send 'G' via Serial               │  2. Open Serial Monitor
   │  3. Drives 30 cm ──────────────────── │
   │  4. Stops                             │
   │  5. Scans 7 angles with HC-SR04       │
   │  6. Sends ESP-NOW packet ────────────►│  3. Packet received ✓
   │                                       │  4. Data printed on screen
```

---

## 🚨 Critical Reminders

| ⚠️ | Rule |
|----|------|
| 🔴 | Never connect 6V/battery directly to ESP32 3.3V |
| 🔴 | Never power servo from ESP32 3.3V |
| 🔴 | HC-SR04 ECHO needs voltage divider (5V → 3.3V) |
| 🟡 | Remove ENA/ENB jumpers from L298N |
| 🟡 | All GNDs must be connected together |
| 🟡 | Lift robot off ground before every motor test |
| 🟢 | ESP32 ISR functions must be marked `IRAM_ATTR` |
| 🟢 | Always use `noInterrupts()` when reading encoder counters |

---

## 📡 ESP-NOW Setup (Stage 8)

```
Step 1: Friend uploads helper_get_mac.ino to Bot 2
Step 2: Friend reads MAC from Serial Monitor (e.g. AA:BB:CC:DD:EE:FF)
Step 3: Friend sends MAC to you
Step 4: You paste MAC into stage8_espnow_tx.ino → BOT2_MAC[]
Step 5: Upload stage8_espnow_tx.ino to Bot 1
Step 6: Friend uploads stage8_espnow_rx.ino to Bot 2
Step 7: Run demo
```

> ESP-NOW is **peer-to-peer** — no Wi-Fi router needed. Range ~200m open air.

---

*Last updated: 2026-09-11 | Arduino IDE 2.3.10 | ESP32 Arduino Core by Espressif*
