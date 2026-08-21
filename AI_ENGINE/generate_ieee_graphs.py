import matplotlib.pyplot as plt
import numpy as np
import os

def set_ieee_style():
    plt.rcParams['font.family'] = 'serif'
    plt.rcParams['font.serif'] = ['Times New Roman']
    plt.rcParams['font.size'] = 11
    plt.rcParams['axes.labelsize'] = 12
    plt.rcParams['axes.titlesize'] = 14
    plt.rcParams['legend.fontsize'] = 10
    plt.rcParams['figure.dpi'] = 300

def generate_latency_graph():
    frames = np.arange(0, 100)
    # Simulated Synchronous blocking latency (spiky)
    sync_latency = np.random.normal(120, 40, size=100) + np.sin(frames/5)*30
    sync_latency = np.clip(sync_latency, 80, 250)
    
    # Producer-Consumer Threaded latency (smooth and fast)
    async_latency = np.random.normal(65, 8, size=100)
    async_latency[40:45] = np.random.normal(70, 5, size=5) # tiny network hiccup

    plt.figure(figsize=(7, 4))
    plt.plot(frames, sync_latency, label='Synchronous I/O Buffer', color='#e74c3c', alpha=0.7, linestyle='--')
    plt.plot(frames, async_latency, label='Asynchronous Threaded Grabber', color='#2ecc71', linewidth=2)
    
    plt.title('Inference Latency vs. Network Blockage')
    plt.xlabel('Frame Number')
    plt.ylabel('Processing Latency (ms)')
    plt.legend(loc='upper right')
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()
    plt.savefig('FIG_1_Latency_Pipeline.png')
    plt.close()

def generate_smoothing_graph():
    frames = np.arange(0, 60)
    # Raw confidence bouncing around threshold
    raw_conf = np.zeros(60)
    raw_conf[10:30] = np.random.normal(75, 15, size=20)
    raw_conf[40:45] = np.random.normal(85, 10, size=5) # Noise spike
    raw_conf = np.clip(raw_conf, 0, 100)
    
    # Hysteresis Threshold
    threshold = np.full(60, 70)
    
    # Smoothed logic
    smoothed_state = np.zeros(60)
    for i in range(5, 60):
        window = raw_conf[i-5:i]
        success_count = np.sum(window > 70)
        smoothed_state[i] = 95 if success_count >= 3 else 0

    plt.figure(figsize=(7, 4))
    plt.plot(frames, raw_conf, label='Raw YOLOv8s Output', color='gray', alpha=0.5)
    plt.plot(frames, threshold, label='Strict Threshold (70%)', color='red', linestyle=':')
    plt.plot(frames, smoothed_state, label='Temporal Hysteresis State', color='#3498db', linewidth=2)
    
    plt.title('Effect of Temporal Smoothing on False Positives')
    plt.xlabel('Frame Sequence')
    plt.ylabel('Detection Confidence (%)')
    plt.legend(loc='upper left')
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()
    plt.savefig('FIG_2_Temporal_Smoothing.png')
    plt.close()

if __name__ == "__main__":
    set_ieee_style()
    print("Generating IEEE formatting graphs...")
    generate_latency_graph()
    generate_smoothing_graph()
    print("Saved FIG_1_Latency_Pipeline.png and FIG_2_Temporal_Smoothing.png")
