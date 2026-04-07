# ISBCAPD: System Architecture & Data Flow

## 🏛️ 1. High-Level Architecture
The **ISBCAPD** system follows a distributed **Edge-Vision-Cloud** hierarchy. This design ensures low-latency detection while maintaining a global tactical overview.

```mermaid
graph TD
    subgraph "EDGE NODES (Physical Bots)"
        E1[ESP32-CAM] -- "MJPEG Stream" --> AE
        E1 -- "Status / RSSI" --> FB
        R1[24GHz Radar] -- "Serial Data" --> E1
    end

    subgraph "INFERENCE SERVER (AI Engine)"
        AE[YOLOv8s ONNX] -- "Person Detected" --> FB
        AE -- "Annotated Stream" --> DS
    end

    subgraph "CLOUD LAYER (Firebase)"
        FB[(Realtime DB)]
    end

    subgraph "C4ISR DASHBOARD (Command & Control)"
        DS[Tactical UI] -- "Sync" --> FB
        DS -- "Live View" --> AE
    end

    %% Styling
    style E1 fill:#070f17,stroke:#00cfff,color:#b8dff0
    style AE fill:#070f17,stroke:#00ff88,color:#b8dff0
    style FB fill:#070f17,stroke:#bf00ff,color:#b8dff0
    style DS fill:#070f17,stroke:#ffaa00,color:#b8dff0
```

## 📡 2. Data Pipeline Design
### A. The "Producer-Consumer" Flow
1. **Producer**: Dedicated thread drains the MJPEG buffer from the ESP32-CAM at 30 FPS.
2. **Buffer**: A thread-safe `Lock` mechanism holds the latest frame.
3. **Consumer**: The AI Engine pulls frames for inference at a variable rate (8-12 FPS).
4. **Broadcast**: Results are pushed to Firebase and the local Flask restreamer simultaneously.

## 🛡️ 3. Security Hardening
The system implements a **Single-Direction Data Push** to Firebase, ensuring that the bot fleet remains "Invisible" to the public web. All database nodes are locked behind a rigorous "Read/Write: False" security rule set in production mode.

---
**Architectural Specification for the ISBCAPD Research Project.**
