"""
LOCAL SENSOR SERVER — Feeds realistic Bot 1 data to the Dashboard
Run: python local_sensor_server.py
Dashboard connects to: http://localhost:5001/data
"""
from flask import Flask, jsonify
from flask_cors import CORS
import random, time

app = Flask(__name__)
CORS(app)

start_time = time.time()
total_dist = 0.0

@app.route('/data')
def get_data():
    global total_dist
    elapsed = time.time() - start_time
    total_dist += random.uniform(5, 25)

    presence = random.random() < 0.15
    moving = presence and random.random() < 0.7
    stationary = presence and not moving

    return jsonify({
        "presence": presence,
        "moving": moving,
        "stationary": stationary,
        "move_dist": random.randint(80, 300) if moving else 0,
        "stat_dist": random.randint(100, 400) if stationary else 0,
        "move_signal": random.randint(40, 95) if moving else 0,
        "stat_signal": random.randint(30, 80) if stationary else 0,
        "bot_moving": True,
        "motor_rpm": random.randint(60, 85),
        "tof_distance_mm": random.randint(200, 2000),
        "total_dist_mm": round(total_dist, 1),
        "gps_lat": 12.9716 + random.uniform(-0.0001, 0.0001),
        "gps_lng": 77.5946 + random.uniform(-0.0001, 0.0001),
        "bot_mode": "EXPLORING",
        "uptime_s": int(elapsed)
    })

if __name__ == '__main__':
    print("=" * 50)
    print("  BOT 1 LOCAL SENSOR SERVER RUNNING")
    print("  Connect dashboard to: http://localhost:5001")
    print("=" * 50)
    app.run(host='0.0.0.0', port=5001, debug=False)
