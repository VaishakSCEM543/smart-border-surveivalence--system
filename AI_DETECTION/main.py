"""
╔══════════════════════════════════════════════════════════════════╗
║  SENTINEL AI — Real-Time Person Detection Engine                ║
║  Architecture: Producer-Consumer Threading + ONNX Runtime       ║
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
from ultralytics import YOLO

# ╔══════════════════════════════════════════╗
# ║  CONFIGURATION                           ║
# ╚══════════════════════════════════════════╝

# ── DEMO TOGGLE ──────────────────────────────
# Set True  = Use laptop webcam (reliable, zero lag — for presentations)
# Set False = Use ESP32-CAM hardware stream (real edge hardware proof)
USE_LAPTOP_CAM = True

# ── HARDWARE ─────────────────────────────────
ESP32_STREAM_URL = "http://10.142.196.205:81/stream"

# ── AI PARAMETERS ────────────────────────────
CONFIDENCE_THRESHOLD = 0.70       # Only fire if AI is >70% certain
RESIZE_DIM = (640, 480)           # Full VGA for inference clarity
TARGET_PROCESS_FPS = 8            # ~8 AI evaluations per second

# ── TEMPORAL SMOOTHING ───────────────────────
SMOOTHING_WINDOW = 5              # Track last N inference results
SMOOTHING_THRESHOLD = 3           # Require N/WINDOW positive frames to confirm

# ── FIREBASE ─────────────────────────────────
FIREBASE_URL = "https://rob-track-default-rtdb.firebaseio.com"
FIREBASE_ENABLED = True

# ── LOGGING ──────────────────────────────────
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.StreamHandler()
    ]
)
logger = logging.getLogger("SENTINEL")


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
                # Laptop webcam
                self.cap = cv2.VideoCapture(self.source)
            else:
                # ESP32 MJPEG stream
                os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = "timeout;5000"
                self.cap = cv2.VideoCapture(self.source, cv2.CAP_FFMPEG)

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
    print("""
╔══════════════════════════════════════════════════════════════╗
║  SENTINEL AI v2.0 — Production-Grade Person Detection       ║
║  Architecture: Producer-Consumer Threading + ONNX Runtime   ║
╚══════════════════════════════════════════════════════════════╝
    """)

    # ── STAGE 1: Model Initialization ──
    logger.info("[STAGE 1] Loading YOLOv8s neural network...")

    # Export to ONNX for 25-40% CPU speedup (one-time operation)
    onnx_path = "yolov8s.onnx"
    if not os.path.exists(onnx_path):
        logger.info("First run: Exporting YOLOv8s → ONNX for optimized inference...")
        base_model = YOLO("yolov8s.pt")
        base_model.export(format="onnx", imgsz=640, simplify=True)
        logger.info("ONNX export complete.")

    model = YOLO(onnx_path)
    logger.info("YOLOv8s ONNX model loaded successfully.")

    # ── STAGE 2: Video Source Selection ──
    if USE_LAPTOP_CAM:
        source = 0
        logger.info("[SOURCE] Laptop webcam selected (demo mode)")
    else:
        source = ESP32_STREAM_URL
        logger.info(f"[SOURCE] ESP32-CAM hardware: {ESP32_STREAM_URL}")

    # ── STAGE 3: Start Producer Thread ──
    logger.info("[STAGE 2] Starting Frame Grabber thread...")
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

    # ── STAGE 4: Detection Loop ──
    logger.info("[STAGE 3] AI Detection Engine active. Press 'q' to stop.\n")

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
            time.sleep(0.01)
            continue

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
            cv2.rectangle(display_frame, (bx1, by1), (bx2, by2), (255, 0, 255), 3)
            label = f"HUMAN {int(bconf * 100)}%"
            cv2.putText(display_frame, label, (bx1, max(by1 - 10, 0)),
                        cv2.FONT_HERSHEY_DUPLEX, 0.6, (255, 0, 255), 2)

        # HUD overlay
        mode = "WEBCAM" if USE_LAPTOP_CAM else "ESP32"
        cv2.putText(display_frame, f"SENTINEL AI | {mode} | {int(fps)} FPS", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 0), 2)

        status = "THREAT" if was_stable_detection else "CLEAR"
        status_color = (0, 0, 255) if was_stable_detection else (0, 255, 0)
        cv2.putText(display_frame, f"Status: {status}", (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, status_color, 2)

        # Telemetry line
        if frame_count % 200 == 0:
            logger.info(f"Telemetry | FPS: {fps:.1f} | Detections: {detect_count} | Frames: {frame_count} | Drops: {grabber.drop_count}")

        cv2.imshow("SENTINEL AI — Person Detection", display_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # ── Graceful Shutdown ──
    logger.info("Shutting down SENTINEL AI...")
    send_detection_to_firebase(False, 0)
    grabber.stop()
    cv2.destroyAllWindows()
    logger.info("System offline. All resources released.")


if __name__ == "__main__":
    main()
