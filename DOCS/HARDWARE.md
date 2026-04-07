# ISBCAPD Hardware Technical Specification (BOM)

## 📡 1. PRIMARY CONTROLLER & VISION
- **Microcontroller**: Espressif Systems ESP32-S (AI-capable SoC)
- **Optical Sensor**: ESP32-CAM (OV2640 / OV3660)
- **Lens**: 160° Wide-angle (Recommended for wide-area patrolling)
- **Protocol**: HTTP/MJPEG over 2.4GHz IEEE 802.11 b/g/n

## 📡 2. SENSOR FUSION ARRAY
### A. HLK-LD2410C (24GHz mmWave Radar)
- **Technology**: FMCW (Frequency Modulated Continuous Wave)
- **Frequency**: 24.0 GHz - 24.25 GHz
- **Interface**: UART / GPIO
- **Detection Range**: 0.75m – 6.0m
- **Purpose**: Static & dynamic target presence detection (Shadow mitigation)

### B. GY-VL53L0XV2 (ToF Sensor)
- **Technology**: Time-of-Flight (VCSEL Laser)
- **Range**: 2.0m (Indoor)
- **Interface**: I2C
- **Purpose**: Obstacle avoidance and ground proximity

## ⚙️ 3. ACTUATION & KINETICS
- **Motors**: N20 6V 200RPM DC Micro Motors
- **Feedback**: Integrated magnetic hall-effect encoders
- **Wheels**: White 3PI miniQ 34mm
- **Chassis**: Custom laser-cut PMMA / 3D-printed ABS

## ⚡ 4. POWER MANAGEMENT
- **Battery**: 3.7V - 7.4V Li-Po (1000mAh min)
- **Regulation**: LDO / Buck converter to stable 3.3V/5.0V rails
- **Peak Current**: ~800mA (WiFi + AI Inference spike)

## 🔌 5. SENSOR INTERFACE & GPIO MAPPING
The following pinout configuration is utilized for the primary bot fleet integration:

| Sensor | ESP32 Pin | Logic | Role |
|--------|-----------|-------|------|
| **HLK-LD2410C (RX)** | GPIO 14 | UART | Motion Signal |
| **HLK-LD2410C (TX)** | GPIO 15 | UART | Data/Commands |
| **GY-VL53L0X (SDA)** | GPIO 13 | I2C | Distance Data |
| **GY-VL53L0X (SCL)** | GPIO 12 | I2C | I2C Clock |

---
**Architectural Blueprint for the ISBCAPD Academic Research Project.**
