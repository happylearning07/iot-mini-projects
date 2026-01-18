#!/usr/bin/env python3
"""
LoRa Sensor API Server

Accepts JSON data from LoRa receiver via HTTP POST and stores it.
"""

import socket
import csv
import json
import logging
import time
from datetime import datetime
from pathlib import Path
from flask import Flask, request, jsonify

app = Flask(__name__)

# Configuration
DATA_DIR = Path("data")
DATA_DIR.mkdir(exist_ok=True)
CSV_FILE = DATA_DIR / "sensor_data.csv"
JSONL_FILE = DATA_DIR / "sensor_data.jsonl"

# Logging setup
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def init_csv():
    """Initialize CSV file with headers if needed."""
    if not CSV_FILE.exists():
        headers = [
            "timestamp", "device_id", "sequence", "uptime", "temperature", "humidity",
            "pressure", "iaq", "iaq_accuracy", "iaq_label", "static_iaq", "co2_ppm", 
            "voc_ppm", "gas_percent", "stabilized", "run_in_complete", "rssi", "snr"
        ]
        with open(CSV_FILE, "w", newline='') as f:
            writer = csv.writer(f)
            writer.writerow(headers)

def save_data(data):
    """Save received data to CSV and JSONL."""
    timestamp = datetime.now().isoformat()
    data["timestamp"] = timestamp

    # Save to JSONL
    with open(JSONL_FILE, "a") as f:
        f.write(json.dumps(data) + "\n")

    # Save to CSV
    row = [
        timestamp,
        data.get("device_id"),
        data.get("sequence"),
        data.get("uptime"),
        data.get("temperature"),
        data.get("humidity"),
        data.get("pressure"),
        data.get("iaq"),
        data.get("iaq_accuracy"),
        data.get("iaq_label"),
        data.get("static_iaq"),
        data.get("co2_ppm"),
        data.get("voc_ppm"),
        data.get("gas_percent"),
        data.get("stabilized"),
        data.get("run_in_complete"),
        data.get("rssi"),
        data.get("snr")
    ]
    with open(CSV_FILE, "a", newline='') as f:
        writer = csv.writer(f)
        writer.writerow(row)

@app.route('/api/sensor', methods=['POST'])
def receive_sensor_data():
    try:
        data = request.json
        if not data:
            return jsonify({"error": "No JSON data provided"}), 400
        
        save_data(data)
        
        logger.info(f"Received Packet #{data.get('sequence')} | "
                    f"Temp: {data.get('temperature')}C | "
                    f"IAQ: {data.get('iaq')} ({data.get('iaq_label')})")
        
        return jsonify({"status": "success", "message": "Data received"}), 200
        
    except Exception as e:
        logger.error(f"Error processing request: {e}")
        return jsonify({"error": str(e)}), 500

def get_ip_address():
    """Get the local IP address of this machine."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

if __name__ == '__main__':
    init_csv()
    host_ip = get_ip_address()
    port = 5000
    
    print("\n" + "="*50)
    print(f" Sensor API Server Running")
    print(f" Address: http://{host_ip}:{port}")
    print(f" Endpoint: http://{host_ip}:{port}/api/sensor")
    print(f" Data directory: {DATA_DIR.absolute()}")
    print("="*50 + "\n")
    
    app.run(host='0.0.0.0', port=port)
