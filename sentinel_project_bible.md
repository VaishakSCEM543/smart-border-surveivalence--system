# SENTINEL — Complete Project Context File
### Last Updated: 2026-04-03

*Share this single file with any AI to instantly restore full project context.*

---

## 1. PROJECT OVERVIEW

**Project Name:** SENTINEL — Autonomous Surveillance Bot with Real-Time AI Detection  
**Team Members:** Vaish + team  
**Purpose:** IEEE paper presentation + working hardware demo  
**Architecture:** Edge sensor node (ESP32-CAM) → AI inference (laptop) → Cloud telemetry (Firebase) → Web dashboard (browser)

---

## 2. FILE LOCATIONS

| File | Path | Purpose |
|------|------|---------|
| **Python AI Script** | `c:\Users\vaish\PycharmProjects\BOTCAM1\main.py` | YOLOv8s person detection engine with threading, ONNX, Firebase push |
| **ONNX Model** | `c:\Users\vaish\PycharmProjects\BOTCAM1\yolov8s.onnx` | Auto-exported optimized AI model (42.8MB) |
| **PyTorch Model** | `c:\Users\vaish\PycharmProjects\BOTCAM1\yolov8s.pt` | Original YOLOv8s weights (22MB) |
| **SENTINEL Dashboard** | `d:\MAJOR PROJECT\BOT_WEBSITE_FINAL_1.html` | 1400+ line single-file web dashboard with Firebase integration |
| **ESP32 Firmware** | `d:\MAJOR PROJECT\BOT_CAM_1\` | Arduino project for ESP32-CAM (CameraWebServer) |
| **Simple Dashboard** | `c:\Users\vaish\PycharmProjects\BOTCAM1\dashboard\` | Earlier minimal Firebase dashboard (index.html, app.js, styles.css, firebase-config.js) |

---

## 3. HARDWARE COMPONENTS

| Component | Model | Purpose | Status |
|-----------|-------|---------|--------|
| Microcontroller | ESP32 | Main bot brain | Working |
| Camera | ESP32-CAM (OV3660) | MJPEG video stream over Wi-Fi | Working |
| Radar Sensor | HI-LINK HLK-LD2410C-P | 24GHz mmWave motion detection | UI Integrated (Simulated) |
| ToF Sensor | REES52 GY-VL53L0XV2 | Obstacle avoidance (NOT mapping) | Not yet integrated |
| Motors | N20 6V 200RPM with Encoder | Bot movement with odometry | Not yet integrated |
| Wheels | White 3PI miniQ 34mm | For N20 motors | Available |

**Important Hardware Limitations:**
- ESP32-CAM is a **single-socket device** — only one client can stream at a time
- ESP32-CAM only supports **2.4GHz Wi-Fi** (no 5GHz)
- ESP32-CAM IP address **changes on every reboot** — always check Serial Monitor
- Camera settings (VGA, Quality) are stored in **RAM** — lost on power cycle
- VL53L0X ToF is a **single-point sensor** (~2m range) — call it "obstacle avoidance" in the paper, NOT "mapping"

---

## 4. ESP32-CAM CONFIGURATION

**Wi-Fi Credentials (in firmware):**
- SSID: `[REDACTED_SSID]`
- Password: `[REDACTED_FOR_GITHUB]`

**Optimal Camera Settings (set via browser at `http://<IP>/`):**
- Resolution: **VGA (640×480)**
- Quality: **25** (high compression to prevent Wi-Fi lag)
- V-Flip: **ON**

**Stream URL format:** `http://<IP>:81/stream`  
**Last known IP:** `10.142.196.205` (changes on reboot)

---

## 5. PYTHON AI SCRIPT — main.py

### Architecture: Producer-Consumer Threading

```
Thread 1 (FrameGrabber)     Thread Main (AI + Render)
─────────────────────       ─────────────────────────
cap.read() in tight loop    Reads latest_frame via Lock
Overwrites shared frame     Runs YOLOv8s ONNX inference
Drains network buffer       Temporal smoothing (3/5 frames)
Never blocks                Renders bounding boxes
                            Pushes to Firebase on state change
```

### Key Configuration Variables (top of main.py):
```python
USE_LAPTOP_CAM = True          # True=webcam, False=ESP32
ESP32_STREAM_URL = "http://10.142.196.205:81/stream"
CONFIDENCE_THRESHOLD = 0.70    # 70% minimum certainty
RESIZE_DIM = (640, 480)        # Full VGA for inference
TARGET_PROCESS_FPS = 8         # AI evaluations per second
SMOOTHING_WINDOW = 5           # Track last 5 frames
SMOOTHING_THRESHOLD = 3        # Need 3/5 positive to confirm
FIREBASE_URL = "https://rob-track-default-rtdb.firebaseio.com"
```

### Dependencies:
```
opencv-python, ultralytics, numpy, torch, torchvision, onnx, onnxruntime, onnxslim
```
**Python version:** 3.13 (Global)

### How to Run:
```bash
python "c:\Users\vaish\PycharmProjects\BOTCAM1\main.py"
```

---

## 6. FIREBASE CONFIGURATION

**Project:** rob-track  
**Console:** https://console.firebase.google.com/project/rob-track  
**RTDB URL:** https://rob-track-default-rtdb.firebaseio.com

### Web SDK Config:
```javascript
const firebaseConfig = {
  apiKey: "REPLACE_WITH_YOUR_FIREBASE_API_KEY",
  authDomain: "REPLACE_WITH_YOUR_FIREBASE_AUTH_DOMAIN",
  databaseURL: "REPLACE_WITH_YOUR_FIREBASE_DATABASE_URL",
  projectId: "REPLACE_WITH_YOUR_FIREBASE_PROJECT_ID",
  storageBucket: "REPLACE_WITH_YOUR_FIREBASE_STORAGE_BUCKET",
  messagingSenderId: "REPLACE_WITH_YOUR_FIREBASE_MESSAGING_SENDER_ID",
  appId: "REPLACE_WITH_YOUR_FIREBASE_APP_ID",
  measurementId: "REPLACE_WITH_YOUR_FIREBASE_MEASUREMENT_ID"
};
```

### Database Structure:
```
rob-track-default-rtdb
└── bot1
    ├── gps
    │   ├── latitude: 12.86649
    │   ├── longitude: 74.928832
    │   ├── satellites: 16
    │   └── timestamp: "726399"
    ├── ai
    │   ├── personDetected: true/false
    │   ├── confidence: 0-100
    │   └── timestamp: "HH:MM:SS"
    ├── radar
    │   └── presence: true/false
    └── camera
        └── stream_url: "http://<ESP32_IP>:81/stream"
```

### Python writes to Firebase via REST API (no Admin SDK needed):
```python
# PUT to https://rob-track-default-rtdb.firebaseio.com/bot1/ai.json
```

---

## 7. SENTINEL DASHBOARD — BOT_WEBSITE_FINAL_1.html

**Location:** `d:\MAJOR PROJECT\BOT_WEBSITE_FINAL_1.html`  
**Login:** admin / 1234  
**Tech:** Single-file HTML + CSS + JS (no build tools needed)

### Features:
- Login screen with animated pulsing rings
- Dark tactical map (Leaflet/OpenStreetMap) with bot markers
- AI Threat Monitor panel (CLEAR / CAUTION / THREAT)
- Bot detail modals with camera, GPS, radar, AI, event log panels
- **Radar Deep-Dive Terminal:** Full-screen diagnostic overlay for BOT-01
  - 360° Circular polar visualization with 6m range rings
  - Real-time Macro/Micro energy VU meters
  - 9-Gate sensitivity heatmap
  - 60-second rolling energy trend chart
- Network Units drawer with search/filter
- 10 bot cards (BOT-01 = LIVE hardware, BOT-02 to BOT-10 = simulated fleet)

### Firebase Integration:
- Uses Firebase compat SDK (v10 CDN)
- 4 real-time `onValue()` listeners bound to `bot1/`:
  - `bot1/gps` → Map marker + coordinates
  - `bot1/ai` → Threat card + confidence bar + detection feed
  - `bot1/radar` → Radar presence status
  - `bot1/camera` → Stream URL auto-populate

---

## 8. OPERATIONAL PROCEDURES

### Starting the Full System:
1. Open `BOT_WEBSITE_FINAL_1.html` in browser → Login
2. Run `python main.py` in terminal
3. Both screens update in real-time via Firebase

### Switching Camera Source:
- Edit `main.py` line 28: `USE_LAPTOP_CAM = True` (webcam) or `False` (ESP32)

### When ESP32-CAM Won't Connect:
1. Check if IP changed (Serial Monitor after pressing RST button)
2. Close ALL browser tabs pointing to ESP32 IP
3. Power-cycle the ESP32 (unplug USB, wait 3 seconds, replug)
4. Update `ESP32_STREAM_URL` in main.py with new IP

### Configuring ESP32 Camera Settings:
1. Navigate to `http://<IP>/` in browser
2. Click "Start Stream"
3. Set Resolution → VGA (640×480)
4. Set Quality → 25
5. **Close the browser tab before running Python script!**

---

## 9. IEEE PAPER NOTES

### Key Technical Contributions:
1. Producer-Consumer threaded architecture for lag-free edge AI
2. ONNX Runtime optimization for CPU inference (25-40% speedup)
3. Temporal smoothing to eliminate detection flicker
4. Cost-effective surveillance using $5 ESP32-CAM vs $200 Jetson Nano

### Recommended Citations:
1. Redmon, J. et al. — "You Only Look Once" (CVPR 2016)
2. Jocher, G. et al. — "Ultralytics YOLOv8" (2023)
3. Espressif Systems — "ESP32 Technical Reference Manual"
4. Zhang, Y. et al. — "ByteTrack" (ECCV 2022)
5. Lin, T. et al. — "Microsoft COCO" (ECCV 2014)

### Metrics to Benchmark:
- End-to-end latency, Inference latency (P50/P95/P99)
- Detection FPS, Video FPS
- False positive/negative rates
- Network throughput, Recovery time

---

## 10. KNOWN ISSUES & FUTURE WORK

### Known Issues:
- ESP32 IP changes on every reboot
- ESP32 single-socket blocks Python when browser is connected
- JPEG artifacts at Quality=25 may reduce low-light accuracy

### Future Work:
- [ ] Integrate HLK-LD2410C radar sensor with ESP32
- [ ] Integrate VL53L0X ToF for obstacle avoidance
- [ ] Integrate N20 motors for autonomous movement
- [ ] Add ByteTrack for person counting/tracking
- [ ] Firebase Admin SDK with service account for secure writes
- [ ] Deploy dashboard to Firebase Hosting
