# Intelligent Swarm based bots for Continuous area Patrolling and detection (ISBCAPD)

![Project Version](https://img.shields.io/badge/Version-3.0_Tactical-cyan)
![Platform](https://img.shields.io/badge/Platform-ESP32--CAM_|_Python_|_Firebase-green)
![Security](https://img.shields.io/badge/Security-Hardened-blueviolet)

## 📡 Overview: Autonomous Area Patrolling
**ISBCAPD** is a high-fidelity, distributed sensor and robotics framework designed for **Continuous area Patrolling and detection**. The system utilizes a swarm of autonomous bots to monitor large perimeters, employing Edge-AI and real-time telemetry to provide a unified command and control interface.

---

### 🖥️ Tactical C4ISR Dashboard
The system's control center is a futuristic, glassmorphism-inspired terminal designed for high-stakes perimeter monitoring:
- **Neural Targeting Matrix**: Live AI-processed video feed from the active swarm.
- **Biometric Analytics**: Real-time confidence scoring and threat classification per unit.
- **Radar Sweep**: Visualized mmWave HLK-LD2410C telemetry for motion tracking.
- **System Vitals**: Real-time bot health, GPS fixes, and connection uptime.

### 🤖 Intelligent AI Backend
- **Edge Vision Engine**: Powered by an optimized **YOLOv8s** model for low-latency person detection.
- **Asynchronous Pipeline**: Built using a Producer-Consumer threading model in Python to eliminate video lag.
- **Temporal Hysteresis Logic**: Multi-frame smoothing algorithm that eliminates false triggers from environmental noise.

### 🛡️ Hardened Cloud Infrastructure
- **Secure Telemetry**: Custom Firebase Realtime Database rules protect swarm data from unauthorized access.
- **Live Sync**: Sub-second synchronization between individual patrol bots and the global dashboard.

---

## 🛠️ System Architecture
- **Vision Core**: Python 3.13 + OpenCV + Ultralytics YOLOv8s + ONNX Runtime
- **Frontend UI**: HTML5 / CSS3 / JavaScript (Share Tech Mono & Orbitron Typography)
- **Communications**: MJPEG HTTP Stream + REST API + Firebase SDK

## 🚀 Deployment Guide

### 1. AI Backend Configuration
```bash
# Clone the repository
git clone https://github.com/VaishakSCEM543/smart-border-surveivalence--system.git
cd smart-border-surveivalence--system

# Install requirements
pip install opencv-python ultralytics onnxruntime flask

# Launch the Primary Detection Engine
python main.py
```

### 2. Tactical Dashboard Access
- Locate and open `BOT_WEBSITE_FINAL_1.html` in your browser.
- Log in with your secure credentials to initiate the drone-fleet sync.

---

## 🔮 4. Future Research Roadmap
As part of our **Major Engineering Project**, we are continuing to develop the ISBCAPD system with:
- **Swarm Coordination**: Decentralized logic for multi-bot patrolling patterns.
- **Enhanced mmWave Filtering**: Refining the HLK-LD2410C data to eliminate background environmental noise.
- **Autonomous Navigation**: Integrating ToF (Time-of-Flight) sensors for dynamic obstacle avoidance in unpredictable terrains.
- **Paper Publication**: Finalizing benchmarks for IEEE journal submission.

## 🤝 5. Research Contributions
We welcome collaboration from the academic and robotics community. Please refer to [CITATION.cff](./CITATION.cff) to formally reference this work in your publications.

---
**"Intelligent Autonomy For Continuous Perimeter Protection."**
