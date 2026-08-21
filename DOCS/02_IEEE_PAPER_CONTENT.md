# Real-Time Human Detection System via Edge Sensor Nodes and Deep Learning

*The following sections are formatted specifically for an IEEE paper or academic technical presentation. You can copy-paste these modules directly into your report.*

## 1. System Architecture Overview
The proposed system utilizes a distributed edge-computing architecture. A low-cost IoT node (ESP32-CAM) serves as the primary visual acquisition hardware, streaming visual telemetry over a local 2.4GHz Wi-Fi network. Instead of processing the extreme computational weight of computer vision on the microcontroller itself, the stream is intercepted by a central computational node (laptop/server) executing a state-of-the-art Deep Convolutional Neural Network (YOLOv8). This methodology bridges the gap between highly constrained hardware sensors and heavy artificial intelligence.

## 2. Hardware and Network Mechanics (ESP32-CAM)
The primary sensor is the OV3660 camera module mounted onto an ESP32 microcontroller. 
By default, the raw visual throughput of a high-resolution sensor will immediately oversaturate the bandwidth limits of a standard 2.4GHz Wi-Fi connection, resulting in severe packet loss and asynchronous video delay (lag). 

To computationally stabilize the network, two hardware-level compression parameters were enforced onto the ESP32 firmware:
1. **Resolution Downscaling:** The resolution was restricted from its maximum UXGA (1600x1200) down to **VGA (640x480)**. This drastically reduced the raw pixel payload per frame by 80%, while maintaining sufficient pixel density for the neural network.
2. **Aggressive JPEG Encoding:** The internal image `Quality` parameter was tuned to 25. This forces the physical hardware on the ESP32 board to heavily zip and compress the image data into a JPEG payload *before* broadcasting it over the network. 

These optimizations allowed the ESP32 to serve a continuous, lightweight **MJPEG (Motion-JPEG)** stream over TCP Port 81 without inducing latency spikes.

## 3. Artificial Intelligence Pipeline (YOLOv8s)
For real-time human recognition, the system deploys **YOLOv8** (You Only Look Once, version 8). 
To combat "hallucinations"—a common phenomenon where AI networks falsely identify shadows, cloth, or animals as humans—the pipeline utilizes the **YOLOv8s (Small)** weights instead of the default Nano version. The deeper mathematical layers of the "Small" version possess the geometric context required to successfully reject non-human shapes.

To configure the model strictly as a security/surveillance entity:
* **Class Isolation:** The model is mathematically constricted to evaluate only Class `0` (Person) out of the 80 default COCO categories.
* **Aggressive Thresholding:** A strict confidence threshold parameter (`conf >= 0.70`) is applied, ensuring bounding boxes are only rendered when the model possesses 70% or greater mathematical certainty of a human signature.

## 4. The "Dual-Loop" Asynchronous Processing Engine (Novel Contribution)
The primary challenge of deploying heavy local AI on live video streams is processing latency. If a neural network requires 125ms to analyze a frame, a traditional synchronous `Read -> Analyze -> Display` loop will inherently bottleneck the stream to an unplayable 8 FPS, causing massive delays.

To resolve this without sacrificing accuracy, an asynchronous **Dual-Loop Engine** was formulated in Python:
* **Loop A (The Video Poller):** This loop runs at maximum CPU frequency. It continuously pulls the heavily compressed MJPEG frames out of the network buffer using OpenCV. By emptying the buffer as fast as the ESP32 broadcasts, network desynchronization (lag) is completely eradicated.
* **Loop B (The AI Evaluator):** Operating independently and slightly slower (e.g., locking to ~8 FPS), this neural loop quietly clones the most recently pulled frame. It passes the matrix through the YOLOv8 layers, generates the spatial coordinates (`X1, Y1, X2, Y2`), and stores them globally. 

Loop A then aggressively renders these locally stored AI tracking coordinates directly over the incoming 30 FPS video feed. This paradigm successfully decouples the high-speed rendering physics from the extreme mathematical weight of the AI, proving that fluid, lag-free human tracking can be achieved over standard Wi-Fi protocols.
