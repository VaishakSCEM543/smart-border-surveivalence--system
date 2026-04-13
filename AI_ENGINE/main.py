"""
╔══════════════════════════════════════════════════════════════════╗
║  ISBCAPD AI v3.0 — Source-Agnostic Person Detection Engine     ║
║  Project: Intelligent Swarm based bots for Continuous area     ║
║           Patrolling and detection                             ║
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

# ╔══════════════════════════════════════════╗
# ║  CONFIGURATION                           ║
# ╚══════════════════════════════════════════╝

# ── SOURCE SELECTION ─────────────────────────
# The AI pipeline is SOURCE-AGNOSTIC: it supports both deployment modes.
# Mode 1 (Tethered):   USE_FALLBACK_CAM = True  → Laptop webcam (high accuracy demo)
# Mode 2 (Autonomous): USE_FALLBACK_CAM = False → ESP32-CAM over WiFi (deployed bot)
USE_FALLBACK_CAM = True

# ── HARDWARE ─────────────────────────────────
ESP32_STREAM_URL = "http://10.25.149.205:81/stream"  # Static IP (set in ESP32 firmware)

# ── AI PARAMETERS ────────────────────────────
CONFIDENCE_THRESHOLD = 0.70       # Only fire if AI is >70% certain
RESIZE_DIM = (640, 480)           # Full VGA for inference clarity
TARGET_PROCESS_FPS = 8            # ~8 AI evaluations per second

# ── TEMPORAL SMOOTHING ───────────────────────
SMOOTHING_WINDOW = 5              # Track last N inference results
SMOOTHING_THRESHOLD = 3           # Require N/WINDOW positive frames to confirm

# ── FIREBASE ─────────────────────────────────
FIREBASE_URL = "REPLACE_WITH_YOUR_FIREBASE_DATABASE_URL"
FIREBASE_ENABLED = True

# ── LOGGING ──────────────────────────────────
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[logging.StreamHandler()]
)
logger = logging.getLogger("ISBCAPD")

# ── RE-STREAM SERVER ─────────────────────────
RESTREAM_PORT = 5000              # Dashboard connects to localhost:5000/feed
RESTREAM_ENABLED = True           # Serve annotated frames via HTTP MJPEG

# ── SHARED STATE FOR RE-STREAM ───────────────
# ── SHARED STATE FOR RE-STREAM ───────────────
latest_annotated_frame = None
annotated_frame_lock = threading.Lock()

# Live telemetry shared with /status endpoint
live_telemetry = {
    'fps': 0, 'detect_count': 0, 'frame_count': 0,
    'threat_active': False, 'confidence': 0, 'uptime': 0,
    'start_time': 0, 'last_alert': 'NONE', 'persons_in_frame': 0
}


# ╔══════════════════════════════════════════╗
# ║  FIREBASE REST API (No SDK Needed)       ║
# ╚══════════════════════════════════════════╝

def firebase_push(path: str, data: dict):
    """Push lightweight JSON data to Firebase RTDB via REST API."""
    if not FIREBASE_ENABLED:
        return
    try:
        url = f"{FIREBASE_URL}/{path}.json"
        body = json.dumps(data).encode('utf-8')
        req = Request(url, data=body, method='PUT')
        req.add_header('Content-Type', 'application/json')
        urlopen(req, timeout=3)
        logger.debug(f"Firebase → {path}: {data}")
    except (URLError, Exception) as e:
        logger.warning(f"Firebase push failed: {e}")


def send_detection_to_firebase(detected: bool, confidence: float = 0):
    """Push AI detection state change to Firebase bot1/ai node."""
    timestamp = time.strftime("%H:%M:%S")
    firebase_push("bot1/ai", {
        "personDetected": detected,
        "confidence": round(confidence * 100) if detected else 0,
        "timestamp": timestamp
    })
    status = "THREAT" if detected else "CLEAR"
    logger.info(f"Firebase AI → {status} (conf: {confidence:.2f})")


# ╔══════════════════════════════════════════╗
# ║  MJPEG RE-STREAM SERVER                  ║
# ║  Serves AI-annotated frames to dashboard ║
# ╚══════════════════════════════════════════╝

class MJPEGHandler(BaseHTTPRequestHandler):
    """HTTP handler that serves the latest annotated frame as MJPEG stream."""

    def do_GET(self):
        if self.path == '/feed':
            self.send_response(200)
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=frame')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
            self.end_headers()

            while True:
                try:
                    with annotated_frame_lock:
                        frame = latest_annotated_frame
                    if frame is None:
                        time.sleep(0.033)
                        continue

                    _, jpeg = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 65])
                    data = jpeg.tobytes()

                    self.wfile.write(b'--frame\r\n')
                    self.wfile.write(b'Content-Type: image/jpeg\r\n')
                    self.wfile.write(f'Content-Length: {len(data)}\r\n'.encode())
                    self.wfile.write(b'\r\n')
                    self.wfile.write(data)
                    self.wfile.write(b'\r\n')
                    
                    time.sleep(0.033)
                except (BrokenPipeError, ConnectionResetError, OSError):
                    break

        elif self.path == '/status':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            status = {
                'online': latest_annotated_frame is not None,
                'source': 'WEBCAM' if USE_FALLBACK_CAM else 'ESP32-CAM',
                'model': 'YOLOv8s-ONNX',
                'restream_fps': 30,
                'inference_fps': round(live_telemetry.get('fps', 0), 1),
                'detect_count': live_telemetry.get('detect_count', 0),
                'frame_count': live_telemetry.get('frame_count', 0),
                'threat_active': live_telemetry.get('threat_active', False),
                'confidence': live_telemetry.get('confidence', 0),
                'uptime_s': round(time.time() - live_telemetry.get('start_time', time.time())),
                'last_alert': live_telemetry.get('last_alert', 'NONE'),
                'persons_in_frame': live_telemetry.get('persons_in_frame', 0)
            }
            self.wfile.write(json.dumps(status).encode())

        elif self.path == '/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(b'<html><body style="background:#000;margin:0;">')
            self.wfile.write(b'<img src="/feed" style="width:100%;height:100vh;object-fit:contain;">')
            self.wfile.write(b'</body></html>')

        else:
            self.send_response(404)
            self.end_headers()

    def log_message(self, format, *args):
        pass  # Suppress HTTP logs

class ReusableThreadingHTTPServer(ThreadingHTTPServer):
    allow_reuse_address = True

def start_restream_server():
    """Start the MJPEG re-stream server in a background thread."""
    server = ReusableThreadingHTTPServer(('0.0.0.0', RESTREAM_PORT), MJPEGHandler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    logger.info(f"[RE-STREAM] MJPEG server live at http://localhost:{RESTREAM_PORT}/feed")
    logger.info(f"[RE-STREAM] Status endpoint at http://localhost:{RESTREAM_PORT}/status")
    return server


# ╔══════════════════════════════════════════╗
# ║  PRODUCER THREAD (Frame Grabber)         ║
# ║  Drains the network buffer continuously  ║
# ╚══════════════════════════════════════════╝

class FrameGrabber(threading.Thread):
    """Dedicated thread that continuously reads frames from the video source."""

    def __init__(self, source):
        super().__init__(daemon=True)
        self.source = source
        self.cap = None
        self.latest_frame = None
        self.lock = threading.Lock()
        self.running = True
        self.connected = False
        self.frame_count = 0
        self.drop_count = 0

    def connect(self):
        """Establish connection to video source with exponential backoff."""
        attempt = 0
        max_retries = 15
        base_delay = 1.0

        while self.running and attempt < max_retries:
            if isinstance(self.source, int):
                # Laptop webcam (tethered mode)
                self.cap = cv2.VideoCapture(self.source)
            else:
                # ESP32 MJPEG stream (autonomous mode)
                os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = "timeout;5000"
                self.cap = cv2.VideoCapture(self.source, cv2.CAP_FFMPEG)
                self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)  # Minimal buffer — always latest frame

            if self.cap.isOpened():
                self.connected = True
                logger.info("Stream connection established.")
                return True

            delay = min(base_delay * (2 ** attempt), 30) * random.uniform(0.8, 1.2)
            logger.warning(f"Connection attempt {attempt+1}/{max_retries} failed. Retry in {delay:.1f}s")
            time.sleep(delay)
            attempt += 1

        logger.error(f"Failed to connect after {max_retries} attempts.")
        return False

    def run(self):
        """Main producer loop — runs in its own thread."""
        if not self.connect():
            return

        while self.running:
            ret, frame = self.cap.read()

            if not ret:
                self.drop_count += 1
                if self.drop_count > 30:
                    logger.warning("Excessive frame drops. Reconnecting...")
                    self.cap.release()
                    self.connected = False
                    if not self.connect():
                        break
                    self.drop_count = 0
                continue

            # Validate frame integrity (corrupted Wi-Fi packets)
            if frame is None or frame.size == 0 or len(frame.shape) < 2:
                continue

            self.drop_count = 0
            self.frame_count += 1

            with self.lock:
                self.latest_frame = frame

    def get_frame(self):
        """Thread-safe frame retrieval for consumer."""
        with self.lock:
            return self.latest_frame.copy() if self.latest_frame is not None else None

    def stop(self):
        self.running = False
        if self.cap:
            self.cap.release()


# ╔══════════════════════════════════════════╗
# ║  MAIN ENGINE                             ║
# ╚══════════════════════════════════════════╝

def main():
    global latest_annotated_frame

    print("""
╔══════════════════════════════════════════════════════════════╗
║  ISBCAPD AI v3.0 — Source-Agnostic Person Detection        ║
║  Project: Intelligent Swarm based bots for Continuous      ║
║           area Patrolling and detection                     ║
║  Architecture: Producer-Consumer + ONNX + MJPEG Re-Stream  ║
║  Modes: Tethered (Webcam) | Autonomous (ESP32-CAM)         ║
╚══════════════════════════════════════════════════════════════╝
    """)

    # ── STAGE 1: Model Initialization ──
    logger.info("[STAGE 1] Loading YOLOv8s neural network...")

    onnx_path = "yolov8s.onnx"
    if not os.path.exists(onnx_path):
        logger.info("First run: Exporting YOLOv8s → ONNX for optimized inference...")
        base_model = YOLO("yolov8s.pt")
        base_model.export(format="onnx", imgsz=640, simplify=True)
        logger.info("ONNX export complete.")

    model = YOLO(onnx_path)
    logger.info("YOLOv8s ONNX model loaded successfully.")

    # ── STAGE 2: Source Selection (Source-Agnostic) ──
    if USE_FALLBACK_CAM:
        source = 0
        source_name = "TETHERED (Laptop Webcam)"
    else:
        source = ESP32_STREAM_URL
        source_name = f"AUTONOMOUS (ESP32-CAM: {ESP32_STREAM_URL})"

    logger.info(f"[SOURCE] {source_name}")

    # ── STAGE 3: Start Re-Stream Server ──
    restream_server = None
    if RESTREAM_ENABLED:
        restream_server = start_restream_server()

    # ── STAGE 4: Start Producer Thread ──
    logger.info("[STAGE 3] Starting Frame Grabber thread...")
    grabber = FrameGrabber(source)
    grabber.start()

    # Wait for first frame
    logger.info("Waiting for first frame...")
    timeout = 60
    start_wait = time.time()
    while grabber.get_frame() is None:
        if time.time() - start_wait > timeout:
            logger.error("Timeout waiting for first frame. Exiting.")
            grabber.stop()
            return
        time.sleep(0.1)

    logger.info("[SUCCESS] Video stream online!")

    # ── STAGE 5: Detection Loop ──
    logger.info("[STAGE 4] AI Detection Engine active. Press 'q' to stop.")
    logger.info(f"[DASHBOARD] Connect to http://localhost:{RESTREAM_PORT}/feed for AI-annotated stream\n")

    # State variables
    last_inference_time = 0
    inference_interval = 1.0 / TARGET_PROCESS_FPS
    prev_frame_time = 0
    fps = 0
    global_boxes = []
    was_stable_detection = False
    detection_history = deque(maxlen=SMOOTHING_WINDOW)
    max_confidence = 0.0
    frame_count = 0
    detect_count = 0
    last_good_frame_time = time.time()
    live_telemetry['start_time'] = time.time()

    # Firebase initial clear
    send_detection_to_firebase(False, 0)

    # Graceful shutdown handler
    shutdown = threading.Event()
    def signal_handler(sig, frame):
        shutdown.set()
    signal.signal(signal.SIGINT, signal_handler)

    while not shutdown.is_set():
        frame = grabber.get_frame()
        if frame is None:
            # ── FRAME WATCHDOG (5s timeout → auto-reconnect) ──
            if time.time() - last_good_frame_time > 5.0:
                logger.warning("Frame watchdog triggered! No frames for 5s. Reconnecting...")
                grabber.stop()
                grabber = FrameGrabber(source)
                grabber.start()
                last_good_frame_time = time.time()
            time.sleep(0.01)
            continue

        last_good_frame_time = time.time()
        current_time = time.time()
        display_frame = frame.copy()
        frame_count += 1

        # ── AI CONSUMER (runs at ~8 FPS) ──
        if (current_time - last_inference_time) >= inference_interval:
            last_inference_time = current_time

            process_frame = cv2.resize(frame, RESIZE_DIM)

            results = model.predict(
                process_frame,
                classes=[0],
                conf=CONFIDENCE_THRESHOLD,
                agnostic_nms=True,
                verbose=False
            )

            is_person_detected = False
            active_boxes = []
            max_conf_this_frame = 0.0

            for result in results:
                for box in result.boxes:
                    is_person_detected = True

                    x1, y1, x2, y2 = map(int, box.xyxy[0])
                    conf = math.ceil((box.conf[0] * 100)) / 100
                    max_conf_this_frame = max(max_conf_this_frame, conf)

                    scale_y = frame.shape[0] / RESIZE_DIM[1]
                    scale_x = frame.shape[1] / RESIZE_DIM[0]

                    orig_x1, orig_y1 = int(x1 * scale_x), int(y1 * scale_y)
                    orig_x2, orig_y2 = int(x2 * scale_x), int(y2 * scale_y)

                    active_boxes.append((orig_x1, orig_y1, orig_x2, orig_y2, conf))

            global_boxes = active_boxes
            max_confidence = max_conf_this_frame

            # ── TEMPORAL SMOOTHING ──
            detection_history.append(is_person_detected)
            positive_count = sum(detection_history)
            is_stable_detection = positive_count >= SMOOTHING_THRESHOLD

            # Only fire Firebase on STATE CHANGE (not every frame)
            if is_stable_detection != was_stable_detection:
                if is_stable_detection:
                    detect_count += 1
                    logger.info(f"▶ PERSON CONFIRMED ({positive_count}/{SMOOTHING_WINDOW} frames, conf: {max_confidence:.2f})")
                else:
                    logger.info(f"▶ AREA CLEARED ({positive_count}/{SMOOTHING_WINDOW} frames)")

                send_detection_to_firebase(is_stable_detection, max_confidence)
                was_stable_detection = is_stable_detection

            # FPS calculation
            fps = 1 / (current_time - prev_frame_time) if prev_frame_time > 0 else 0
            prev_frame_time = current_time

        # ── RENDER ENGINE ──
        for (bx1, by1, bx2, by2, bconf) in global_boxes:
            # Bounding box
            cv2.rectangle(display_frame, (bx1, by1), (bx2, by2), (0, 255, 255), 2)
            # Label background
            label = f"HUMAN {int(bconf * 100)}%"
            (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
            cv2.rectangle(display_frame, (bx1, max(by1 - th - 10, 0)), (bx1 + tw + 6, max(by1, 0)), (0, 255, 255), -1)
            cv2.putText(display_frame, label, (bx1 + 3, max(by1 - 5, 12)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)

        # HUD overlay
        mode = "TETHERED" if USE_FALLBACK_CAM else "AUTONOMOUS"
        cv2.putText(display_frame, f"ISBCAPD AI | {mode} | {int(fps)} FPS", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 0), 2)

        status = "THREAT" if was_stable_detection else "CLEAR"
        status_color = (0, 0, 255) if was_stable_detection else (0, 255, 0)
        cv2.putText(display_frame, f"Status: {status}", (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, status_color, 2)

        # ── EXPORT ANNOTATED FRAME FOR RE-STREAM ──
        with annotated_frame_lock:
            latest_annotated_frame = display_frame.copy()

        # ── UPDATE LIVE TELEMETRY FOR /status ──
        live_telemetry.update({
            'fps': fps,
            'detect_count': detect_count,
            'frame_count': frame_count,
            'threat_active': was_stable_detection,
            'confidence': int(max_confidence * 100),
            'persons_in_frame': len(global_boxes),
            'last_alert': time.strftime('%H:%M:%S') if was_stable_detection else live_telemetry.get('last_alert', 'NONE')
        })

        # Telemetry line
        if frame_count % 200 == 0:
            logger.info(f"Telemetry | FPS: {fps:.1f} | Detections: {detect_count} | Frames: {frame_count} | Drops: {grabber.drop_count}")

        cv2.imshow("ISBCAPD AI — Person Detection", display_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # ── Graceful Shutdown ──
    logger.info("Shutting down ISBCAPD AI...")
    send_detection_to_firebase(False, 0)
    grabber.stop()
    if restream_server:
        restream_server.shutdown()
    cv2.destroyAllWindows()
    logger.info("System offline. All resources released.")


if __name__ == "__main__":
    main()

