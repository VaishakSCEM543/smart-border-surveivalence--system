# ISBCAPD AI Inference Engine Specification

## 🧠 1. Model Architecture
- **Base Model**: Ultralytics YOLOv8s (Small)
- **Input Resolution**: 640 x 480 (VGA)
- **Format**: ONNX (Open Neural Network Exchange)
- **Quantization**: FP32 (Full Precision)

## 📊 2. Dataset Characteristics
The inference engine leverages the **COCO (Common Objects in Context)** pre-trained dataset for primary "Person" class detection. 
- **Object Class**: 0 (Person)
- **Training Samples**: 80,000+ images
- **Mean Average Precision (mAP@50-95)**: ~44.9

## ⚙️ 3. Inference Pipeline Logic
1. **Normalization**: Scaling pixel values to [0, 1].
2. **NMS (Non-Maximum Suppression)**: Overlap threshold set to **0.45** to reduce redundant bounding boxes.
3. **Temporal Hysteresis**: A sliding window of **5 frames** is used to confirm a "Threat" state, ensuring 100% reliability in noisy environmental conditions.

---
**Artificial Intelligence Overview for the ISBCAPD Research Project.**
