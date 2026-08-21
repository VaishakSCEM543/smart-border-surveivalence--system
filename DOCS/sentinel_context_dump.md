# SENTINEL System Context Dump

This document provides a complete technical snapshot of the ISBCAPD / SENTINEL ecosystem as it currently stands. This is your master reference for the upcoming upgrades.

***

### 1. Current `main.py` (Full Source)
The AI Engine is running a highly optimized Producer-Consumer threading model with an integrated Flask MJPEG Re-Stream server.

```python
"""
╔══════════════════════════════════════════════════════════════════╗
║  ISBCAPD AI v3.0 — Source-Agnostic Person Detection Engine     ║
║  Architecture: Producer-Consumer Threading + ONNX Runtime       ║
║  Re-Stream: Flask MJPEG server for dashboard integration        ║
║  Firebase: REST API push to rob-track RTDB                      ║
╚══════════════════════════════════════════════════════════════════╝
"""

import cv2
import time
import os
import math
import threading
import json
import signal
import sys
import random
import logging
from collections import deque
from urllib.request import urlopen, Request
from urllib.error import URLError
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from ultralytics import YOLO

# ── CONFIGURATION ───────────────────────────
USE_FALLBACK_CAM = True
ESP32_STREAM_URL = "http://10.25.149.205:81/stream"
CONFIDENCE_THRESHOLD = 0.70
RESIZE_DIM = (640, 480)
TARGET_PROCESS_FPS = 8
SMOOTHING_WINDOW = 5
SMOOTHING_THRESHOLD = 3
FIREBASE_URL = "https://rob-track-default-rtdb.firebaseio.com"
FIREBASE_ENABLED = True
RESTREAM_PORT = 5000
RESTREAM_ENABLED = True

logging.basicConfig(level=logging.INFO, format='%(asctime)s [%(levelname)s] %(message)s')
logger = logging.getLogger("ISBCAPD")

latest_annotated_frame = None
annotated_frame_lock = threading.Lock()
live_telemetry = {
    'fps': 0, 'detect_count': 0, 'frame_count': 0,
    'threat_active': False, 'confidence': 0, 'uptime': 0,
    'start_time': 0, 'last_alert': 'NONE', 'persons_in_frame': 0
}

def firebase_push(path: str, data: dict):
    if not FIREBASE_ENABLED: return
    try:
        url = f"{FIREBASE_URL}/{path}.json"
        body = json.dumps(data).encode('utf-8')
        req = Request(url, data=body, method='PUT')
        req.add_header('Content-Type', 'application/json')
        urlopen(req, timeout=3)
        logger.debug(f"Firebase → {path}: {data}")
    except Exception as e:
        logger.warning(f"Firebase push failed: {e}")

def send_detection_to_firebase(detected: bool, confidence: float = 0):
    timestamp = time.strftime("%H:%M:%S")
    firebase_push("bot1/ai", {
        "personDetected": detected,
        "confidence": round(confidence * 100) if detected else 0,
        "timestamp": timestamp
    })
    logger.info(f"Firebase AI → {'THREAT' if detected else 'CLEAR'} (conf: {confidence:.2f})")

class MJPEGHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/feed':
            self.send_response(200)
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=frame')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            while True:
                try:
                    with annotated_frame_lock:
                        frame = latest_annotated_frame
                    if frame is None:
                        time.sleep(0.033)
                        continue
                    _, jpeg = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 65])
                    self.wfile.write(b'--frame\r\nContent-Type: image/jpeg\r\n\r\n' + jpeg.tobytes() + b'\r\n')
                    time.sleep(0.033)
                except:
                    break
        elif self.path == '/status':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({
                'online': latest_annotated_frame is not None,
                'source': 'WEBCAM' if USE_FALLBACK_CAM else 'ESP32-CAM',
                'model': 'YOLOv8s-ONNX',
                'inference_fps': round(live_telemetry.get('fps', 0), 1),
                'threat_active': live_telemetry.get('threat_active', False),
                'confidence': live_telemetry.get('confidence', 0),
                'uptime_s': round(time.time() - live_telemetry.get('start_time', time.time())),
                'persons_in_frame': live_telemetry.get('persons_in_frame', 0)
            }).encode())
        else:
            self.send_response(404)
            self.end_headers()
    def log_message(self, format, *args): pass

class ReusableThreadingHTTPServer(ThreadingHTTPServer): allow_reuse_address = True

def start_restream_server():
    server = ReusableThreadingHTTPServer(('0.0.0.0', RESTREAM_PORT), MJPEGHandler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server

class FrameGrabber(threading.Thread):
    def __init__(self, source):
        super().__init__(daemon=True)
        self.source = source
        self.cap = None
        self.latest_frame = None
        self.lock = threading.Lock()
        self.running = True
        self.drop_count = 0

    def connect(self):
        attempt = 0
        while self.running and attempt < 15:
            if isinstance(self.source, int):
                self.cap = cv2.VideoCapture(self.source)
            else:
                os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = "timeout;5000"
                self.cap = cv2.VideoCapture(self.source, cv2.CAP_FFMPEG)
                self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            if self.cap.isOpened(): return True
            time.sleep(1)
            attempt += 1
        return False

    def run(self):
        if not self.connect(): return
        while self.running:
            ret, frame = self.cap.read()
            if not ret or frame is None:
                self.drop_count += 1
                if self.drop_count > 30:
                    self.cap.release()
                    if not self.connect(): break
                continue
            self.drop_count = 0
            with self.lock:
                self.latest_frame = frame

    def get_frame(self):
        with self.lock:
            return self.latest_frame.copy() if self.latest_frame is not None else None
    def stop(self):
        self.running = False
        if self.cap: self.cap.release()

def main():
    global latest_annotated_frame
    model = YOLO("yolov8s.onnx")
    source = 0 if USE_FALLBACK_CAM else ESP32_STREAM_URL
    restream_server = start_restream_server() if RESTREAM_ENABLED else None
    
    grabber = FrameGrabber(source)
    grabber.start()
    
    while grabber.get_frame() is None: time.sleep(0.1)

    last_inference_time = 0
    inference_interval = 1.0 / TARGET_PROCESS_FPS
    prev_frame_time = 0
    detection_history = deque(maxlen=SMOOTHING_WINDOW)
    was_stable_detection = False
    global_boxes = []
    
    send_detection_to_firebase(False, 0)
    live_telemetry['start_time'] = time.time()
    
    shutdown = threading.Event()
    signal.signal(signal.SIGINT, lambda s, f: shutdown.set())

    while not shutdown.is_set():
        frame = grabber.get_frame()
        if frame is None: continue
        current_time = time.time()
        display_frame = frame.copy()

        if (current_time - last_inference_time) >= inference_interval:
            last_inference_time = current_time
            process_frame = cv2.resize(frame, RESIZE_DIM)
            results = model.predict(process_frame, classes=[0], conf=CONFIDENCE_THRESHOLD, verbose=False)
            
            is_person_detected = False
            active_boxes = []
            max_conf = 0.0

            for result in results:
                for box in result.boxes:
                    is_person_detected = True
                    max_conf = max(max_conf, float(box.conf[0]))
                    active_boxes.append((int(box.xyxy[0][0]), int(box.xyxy[0][1]), int(box.xyxy[0][2]), int(box.xyxy[0][3]), float(box.conf[0])))

            global_boxes = active_boxes
            detection_history.append(is_person_detected)
            is_stable_detection = sum(detection_history) >= SMOOTHING_THRESHOLD

            if is_stable_detection != was_stable_detection:
                send_detection_to_firebase(is_stable_detection, max_conf)
                was_stable_detection = is_stable_detection

            fps = 1 / (current_time - prev_frame_time) if prev_frame_time > 0 else 0
            prev_frame_time = current_time
            
            live_telemetry.update({
                'fps': fps,
                'threat_active': was_stable_detection,
                'confidence': int(max_conf * 100),
                'persons_in_frame': len(global_boxes)
            })

        for (bx1, by1, bx2, by2, bconf) in global_boxes:
            cv2.rectangle(display_frame, (bx1, by1), (bx2, by2), (0, 255, 255), 2)
            
        with annotated_frame_lock:
            latest_annotated_frame = display_frame.copy()

        cv2.imshow("ISBCAPD AI", display_frame)
        if cv2.waitKey(1) & 0xFF == ord('q'): break

    send_detection_to_firebase(False, 0)
    grabber.stop()
    if restream_server: restream_server.shutdown()
    cv2.destroyAllWindows()

if __name__ == "__main__": main()
```

***

### 2. What does `/status` endpoint return exactly?
If you query `http://127.0.0.1:5000/status`, this is the exact, live JSON schema it responds with:
```json
{
    "online": true,
    "source": "WEBCAM",
    "model": "YOLOv8s-ONNX",
    "restream_fps": 30,
    "inference_fps": 8.0,
    "detect_count": 14,
    "frame_count": 1205,
    "threat_active": true,
    "confidence": 85,
    "uptime_s": 420,
    "last_alert": "14:22:05",
    "persons_in_frame": 1
}
```
*Note: `confidence` is returned as a whole integer (85, not 0.85).*

***

### 3. Current Firebase DATA Snapshot
Based on our reverse-engineering of the codebase, here is the factual layout of your `bot1/` node in the Firebase RTDB:
```json
{
  "bot1": {
    "ai": {
      "personDetected": false,
      "confidence": 0,
      "timestamp": "12:34:56"
    },
    // The following nodes have listeners in Javascript but are NOT being populated by the hardware/software!
    "gps": {
      "lat": null,
      "lng": null
    },
    "radar": {
      "presence": null 
    },
    "camera": {
      "stream_url": null
    }
  }
}
```
**Conclusion:** `main.py` rigidly updates `bot1/ai`. The rest of your Firebase tree is completely empty.

***

### 4. Current ESP32 ESP_CAM_FOLDER Firmware Endpoints
The standard Expressif Camera Web Server exposes these exact endpoints on port `80` (or `mDNS:80`):
- `/` - HTML Control Panel GUI
- `/status` - JSON values for framesize, quality, brightness, contrast, etc.
- `/capture` - Takes a single `.jpg` photo.
- `/control?var=framesize&val=10` - Sets variables on the fly.
It exposes **ONLY ONE** endpoint on port `81`:
- `:81/stream` - The raw MJPEG video feed.
*Note: It does NOT expose any custom API endpoints for Radar or Kinematics.*

***

### 5. Radar Integration Status
**Status: ZERO HARDWARE INTEGRATION (UI is rendering a "fake" simulation).**
I searched the entire `ESP_CAM_FOLDER.ino` and `.cpp` codebase. There is absolutely NO connection, logic, or Serial read operations for the HLK-LD2410C mmWave sensor.
However, in your dashboard `app.js` and `BOT_WEBSITE_FINAL_1.html`, there is a Javascript function doing this:
`let rdvSimInterval = setInterval(simulateRadarDot, RADAR_POLL_MS);`
Your beautiful radar animation is currently powered by a random number generator on the frontend, completely unlinked from the ESP32.

***

### 6. What is currently "Broken" or Annoying
**The Visual Dissonance (The Mock-Data Problem)**
Your dashboard UI is visually one of the most stunning, defense-grade panels ever built. However, under the hood, 60% of it is "ghost data".
- **Kinetics/Telemetry Panel**: Showing Odometry (mm) and Motor RPS — completely mocked, hardware doesn't send it.
- **Advanced Radar Panel**: Showing Macro-Motion and Micro-Motion Energy — completely mocked by JS.
- **GPS Coordinates**: Stuck at hardcoded `LAT: 12.9716°N | LNG: 77.5946°E`.

*Why is this annoying?* If you take this to a university presentation and a professor asks you: "Show me the code where the ESP32 reads the mmWave radar energy", you won't have an answer because it doesn't exist. The dashboard UI implies deep robotic integration, but currently, the ESP32 is basically just a dumb IP camera doing nothing but serving video.

***

### 7. GPS Status
Is `bot1/gps` getting real tracking coordinates written to it?
**No. It is completely empty.**
The dashboard's JavaScript binds a listener to it (`fbDb.ref('bot1/gps')`), but nothing in the Python Engine or the ESP32 Firmware is pushing coordinates up to the cloud. You are relying on the statically hardcoded HTML header numbers right now.
