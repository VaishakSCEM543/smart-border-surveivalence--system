# 🎯 ISBCAPD — Full Q&A Cheat Sheet
## Paper Presentation Prep | 24 Sep 2026

---

## 🤖 SECTION 1 — AI Model Questions

---

### Q1. What AI model did you use and why YOLOv8?
> **Answer:** We used **YOLOv8s (small variant)** by Ultralytics. We chose it because:
> - It is the best balance of **speed vs accuracy** for real-time detection
> - The "s" (small) variant runs well on **CPU — no GPU required**
> - It supports **ONNX export**, which gives 2-3x faster CPU inference than raw PyTorch
> - It detects `person` class out-of-the-box with high accuracy (trained on COCO dataset — 80 classes, 330k images)

---

### Q2. What is ONNX and why did you use it instead of the .pt PyTorch model?
> **Answer:** ONNX (Open Neural Network Exchange) is a universal model format that can be run by **ONNX Runtime** — a highly optimized inference engine. Benefits:
> - **2-3x faster** on CPU vs running `.pt` directly in PyTorch
> - **No GPU needed** — runs on any laptop
> - Framework-independent — works without installing full PyTorch on deployment machine
> - Our model: `yolov8s.onnx` (44MB), auto-exported from `yolov8s.pt` on first run

---

### Q3. What is Temporal Smoothing / Hysteresis? Why is it needed?
> **Answer:** A single YOLO frame can give a **false positive** due to shadows, lighting change, or camera shake. Temporal Smoothing fixes this:
> - We maintain a **sliding window (deque) of last N=5 frames**
> - A threat is only confirmed if **3 out of 5 consecutive frames** show detection above 70% confidence
> - `SMOOTHING_WINDOW = 5`, `SMOOTHING_THRESHOLD = 3` (from the actual code)
> - A single spike resets — only sustained, real detections pass through
> - Firebase is only updated on **state change** (CLEAR → THREAT or THREAT → CLEAR), not every frame

---

### Q4. What confidence threshold did you use and why 70%?
> **Answer:** `CONFIDENCE_THRESHOLD = 0.70` (from code line 42).
> - Below 70% = too many false positives from partial occlusions
> - Above 85% = misses real threats in dim light or at angles
> - 70% is the industry-standard sweet spot for real-time person detection with YOLOv8

---

### Q5. What FPS does your AI run at?
> **Answer:** `TARGET_PROCESS_FPS = 8` — about 8 AI inferences per second (from code line 44).
> - The display/re-stream runs at **30 FPS** (smooth video)
> - YOLO only runs every `1/8 = 125ms` to save CPU
> - The Producer Thread keeps grabbing at full speed — so YOLO always gets the **newest** frame even when it runs slower

---

### Q6. What is the Producer-Consumer architecture?
> **Answer:** Two separate threads:
> - **Producer (FrameGrabber thread):** Runs at full speed. Reads frames from webcam or ESP32-CAM stream. Stores only the **latest frame** in a thread-safe lock buffer. Old frames are discarded.
> - **Consumer (main thread):** Reads the latest frame from the buffer and runs YOLO on it every 125ms
> - **Why:** Without this, YOLO would process frames in order — by the time it finishes inference, it picks up the frame from 500ms ago. With the producer thread, it **always processes the newest frame**, reducing effective latency by ~40%

---

### Q7. How does the AI connect to the dashboard?
> **Answer:** The AI engine runs a **MJPEG re-stream server on port 5000**:
> - After annotating each frame with bounding boxes, it serves it at `http://localhost:5000/feed`
> - The dashboard opens this URL as an `<img src="">` tag — getting a live annotated video feed
> - A `/status` endpoint at `http://localhost:5000/status` returns live JSON (FPS, threat status, confidence, uptime)

---

### Q8. How are threat alerts sent to the cloud?
> **Answer:** Via **Firebase Realtime Database (RTDB)** using the REST API — no SDK needed:
> - On state change → `PUT` request to `https://rob-track-default-rtdb.firebaseio.com/bot1/ai.json`
> - Payload: `{ "personDetected": true, "confidence": 87, "timestamp": "09:15:32" }`
> - The dashboard listens on Firebase in real-time and updates instantly

---

### Q9. Is your system source-agnostic? What does that mean?
> **Answer:** Yes. One flag `USE_FALLBACK_CAM` (line 36) switches the entire pipeline:
> - `True` → Laptop webcam (`cv2.VideoCapture(0)`) — used in our demo
> - `False` → ESP32-CAM stream (`cv2.VideoCapture("http://192.168.x.x:81/stream")`)
> - **Everything after the frame source is identical** — same threading, same YOLO, same smoothing, same Firebase push

---

### Q10. What happens if the camera stream drops / network disconnects?
> **Answer:** The system has a **5-second Frame Watchdog**:
> - If no valid frame arrives for 5 seconds, the grabber thread is stopped and a **new connection is attempted automatically**
> - The `connect()` function uses **exponential backoff** (1s, 2s, 4s... up to 30s) with jitter to avoid reconnect storms
> - Corrupted/empty frames (from bad Wi-Fi packets) are validated and discarded before reaching YOLO

---

## 💻 SECTION 2 — Software Stack Questions

---

### Q11. What is your full software stack?
> **Answer:**

| Layer | Technology | Purpose |
|---|---|---|
| Bot Firmware | C++ / Arduino IDE | Motor control, sensors, ESP-NOW |
| Communication | ESP-NOW + Wi-Fi | Bot-to-bot + bot-to-network |
| AI Engine | Python 3, OpenCV, Ultralytics, ONNX Runtime | Person detection |
| Cloud Backend | Firebase RTDB (REST API) | Real-time event storage |
| Dashboard | HTML + CSS + JavaScript + Firebase JS SDK | Operator UI |
| Re-stream | Python HTTP server (stdlib) | Serve annotated MJPEG to dashboard |

---

### Q12. Why Python for the AI engine and not C++?
> **Answer:** Python has the richest ecosystem for AI/ML:
> - `ultralytics` (YOLOv8), `opencv-python`, `onnxruntime` are all Python-first
> - The ESP32 does the real-time hardware control in C++ where speed matters
> - The base station laptop has enough resources to run Python comfortably
> - **Separation of concerns:** Hardware (C++) handles timing-critical tasks, Software (Python) handles compute-heavy tasks

---

### Q13. Why Firebase and not a custom server?
> **Answer:**
> - **No server setup needed** — Firebase is a managed cloud service
> - **Real-time listeners** — dashboard updates within milliseconds of an event, no polling
> - **Free tier** is sufficient for our data volumes
> - **REST API** — even the Python AI engine can push without an SDK, just `urllib`

---

### Q14. What is ESP-NOW and how is it different from regular Wi-Fi?
> **Answer:**

| | ESP-NOW | Regular Wi-Fi |
|---|---|---|
| Router needed? | ❌ No | ✅ Yes |
| Latency | ~1-3ms | ~15-50ms |
| Range | ~200m open air | ~50m indoors |
| Protocol | Peer-to-peer MAC layer | TCP/IP stack |
| Power | Very low | Higher |

> ESP-NOW works at the **MAC layer directly**, bypassing the TCP/IP stack entirely. This is why it's so fast and low power — ideal for bot-to-bot coordination in the field.

---

### Q15. Why does Bot 1 run both ESP-NOW AND Wi-Fi simultaneously?
> **Answer:** ESP32 supports **Wi-Fi Station mode + ESP-NOW coexistence** on the same radio, as long as both operate on the **same Wi-Fi channel**.
> - Bot 1 connects to the router (for dashboard HTTP endpoint)
> - Bot 1 also sends ESP-NOW packets to Bot 2's MAC address directly
> - The channel is read from `WiFi.channel()` and passed to `peerInfo.channel` to ensure they match

---

### Q16. What does the `/data` endpoint return?
> **Answer:** Each bot runs a `WebServer` on port 80. A GET to `http://<ip>/data` returns:
> ```json
> {
>   "bot_id": 1,
>   "local_distance": 45.23,
>   "local_blocked": false,
>   "peer_blocked": false,
>   "left_pwm": 145,
>   "right_pwm": 138
> }
> ```
> The dashboard polls this every 500ms to show live bot status.

---

### Q17. What is micro-ROS and ROS 2? (from your README — future scope)
> **Answer:** ROS 2 (Robot Operating System 2) is a framework for building robot software. **micro-ROS** is a lightweight version that runs on microcontrollers like ESP32.
> - Currently we use direct HTTP/ESP-NOW for communication
> - In the full deployment, micro-ROS would allow bots to publish sensor data as **ROS topics**, enabling **SLAM (Simultaneous Localization and Mapping)** and **Nav2 (autonomous navigation)**
> - This is listed as a **future scope / next phase** of the project

---

### Q18. What are the limitations of your current system?
> **Answer (be honest — panels respect this):**
> 1. AI runs at only 8 FPS — sufficient for walking humans, not fast vehicles
> 2. YOLOv8s is not fine-tuned on border scenarios — it's COCO general weights
> 3. Bot 2 is a dummy node — in full deployment each bot would have all sensors
> 4. GPS module is present in design but GPS data in the demo is manually entered in Firebase
> 5. Radar telemetry in the demo is **simulated** in Python (the real HLK-LD2410C radar data integration is planned)

---

### Q19. What makes this project novel / what is your contribution?
> **Answer:**
> 1. **Source-agnostic AI pipeline** — one codebase handles both laptop webcam and ESP32-CAM with a single flag
> 2. **Temporal Hysteresis for embedded surveillance** — applying multi-frame smoothing to drastically reduce false positives in edge IoT contexts
> 3. **Producer-Consumer threading** applied to ESP32-CAM's unreliable MJPEG stream — always processes the freshest frame
> 4. **Swarm awareness behavior** — bots automatically adjust speed based on peer's obstacle status via ESP-NOW
> 5. Full stack: from bare-metal C++ firmware to Python AI to cloud Firebase to web dashboard — end to end by a student team
