import cv2
import time
import sys

stream_url = "http://10.25.149.205:81/stream"
print(f"Testing stream connection to {stream_url} ...")

cap = cv2.VideoCapture(stream_url)
if not cap.isOpened():
    print("FAILED: Could not open the video stream. The network connection might be too poor.")
    sys.exit(1)

print("SUCCESS: Stream opened. Measuring framerate over 50 frames...")
frame_count = 0
start_time = time.time()

while frame_count < 50:
    ret, frame = cap.read()
    if not ret:
        print(f"\nWARNING: Frame read failed at frame {frame_count}. Stream dropped.")
        break
    frame_count += 1
    sys.stdout.write(f"\rCaptured {frame_count}/50 frames")
    sys.stdout.flush()

end_time = time.time()
cap.release()

if frame_count > 0:
    fps = frame_count / (end_time - start_time)
    print(f"\nCompleted. Average FPS: {fps:.2f}")
    if fps < 5:
        print("DIAGNOSIS: The framerate is extremely low (<5 FPS). This confirms a poor Wi-Fi signal, power brownout, or antenna issue.")
    else:
        print("DIAGNOSIS: The stream seems to be running relatively smoothly from the Python side.")
else:
    print("\nDIAGNOSIS: Failed to read any frames.")
