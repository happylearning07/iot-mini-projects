#!/usr/bin/env python3
"""
LoRa Sensor API Server - OPTIMIZED

Accepts JSON data from LoRa receiver via HTTP POST and stores it.
Optimized for high-throughput with async I/O and buffering.
"""

import socket
import csv
import json
import logging
import time
import threading
from queue import Queue
from datetime import datetime
from pathlib import Path
from flask import Flask, request, jsonify

app = Flask(__name__)

# Configuration
DATA_DIR = Path("data")
DATA_DIR.mkdir(exist_ok=True)
BSEC_CSV_FILE = DATA_DIR / "bsec_data.csv"
ANALOG_CSV_FILE = DATA_DIR / "analog_data.csv"
JSONL_FILE = DATA_DIR / "sensor_data.jsonl"

# Buffering configuration
BUFFER_SIZE = 100  # Buffer up to 100 records before flushing
FLUSH_INTERVAL = 1.0  # Flush every 1 second

# Logging setup
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Global queues for async I/O
data_queue = Queue()
stats = {
    'received': 0,
    'queued': 0,
    'written': 0,
    'errors': 0
}

def init_csv():
    """Initialize CSV files with headers if needed."""
    # BSEC Data Headers
    if not BSEC_CSV_FILE.exists():
        headers_bsec = [
            "timestamp", "device_id", "sequence", "uptime", "temperature", "humidity",
            "pressure", "iaq", "iaq_accuracy", "iaq_label", "static_iaq", "co2_ppm", 
            "voc_ppm", "gas_percent", "stabilized", "run_in_complete", "rssi", "snr",
            "mq135_raw", "anemometer_raw"
        ]
        with open(BSEC_CSV_FILE, "w", newline='') as f:
            writer = csv.writer(f)
            writer.writerow(headers_bsec)

    # Analog Data Headers
    if not ANALOG_CSV_FILE.exists():
        headers_analog = [
            "timestamp", "device_id", "sequence", "uptime", 
            "mq135_raw", "anemometer_raw", "rssi", "snr"
        ]
        with open(ANALOG_CSV_FILE, "w", newline='') as f:
            writer = csv.writer(f)
            writer.writerow(headers_analog)

def save_data_batch(data_batch):
    """Save a batch of data records to persistent storage."""
    if not data_batch:
        return
    
    try:
        # Prepare batches
        jsonl_lines = []
        bsec_rows = []
        analog_rows = []
        
        for data in data_batch:
            timestamp = data["timestamp"]
            
            # 1. Prepare JSONL line
            jsonl_lines.append(json.dumps(data))
            
            # 2. Prepare BSEC row if temperature is present
            if "temperature" in data:
                bsec_rows.append([
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
                    data.get("snr"),
                    data.get("mq135_raw"),
                    data.get("anemometer_raw")
                ])
            
            # 3. Prepare Analog row if analog data is present
            if "mq135_raw" in data:
                analog_rows.append([
                    timestamp,
                    data.get("device_id"),
                    data.get("sequence"),
                    data.get("uptime"),
                    data.get("mq135_raw"),
                    data.get("anemometer_raw"),
                    data.get("rssi"),
                    data.get("snr")
                ])
        
        # Write all at once (much faster than individual writes)
        if jsonl_lines:
            with open(JSONL_FILE, "a") as f:
                f.write("\n".join(jsonl_lines) + "\n")
        
        if bsec_rows:
            with open(BSEC_CSV_FILE, "a", newline='') as f:
                csv.writer(f).writerows(bsec_rows)
        
        if analog_rows:
            with open(ANALOG_CSV_FILE, "a", newline='') as f:
                csv.writer(f).writerows(analog_rows)
        
        stats['written'] += len(data_batch)
        
    except Exception as e:
        logger.error(f"Error writing batch: {e}")
        stats['errors'] += 1

def background_writer():
    """Background thread that writes data in batches."""
    buffer = []
    last_flush = time.time()
    
    while True:
        try:
            # Try to get data from queue (with timeout)
            try:
                data = data_queue.get(timeout=0.1)
                buffer.append(data)
            except:
                # Queue empty, continue to check if we should flush
                pass
            
            # Flush if buffer is full or enough time has passed
            now = time.time()
            should_flush = (len(buffer) >= BUFFER_SIZE) or \
                          (buffer and (now - last_flush) >= FLUSH_INTERVAL)
            
            if should_flush:
                save_data_batch(buffer)
                buffer = []
                last_flush = now
                
        except Exception as e:
            logger.error(f"Background writer error: {e}")
            time.sleep(0.1)

@app.route('/api/sensor', methods=['POST'])
def receive_sensor_data():
    try:
        data = request.json
        if not data:
            return jsonify({"error": "No JSON data provided"}), 400
        
        stats['received'] += 1
        
        # Add timestamp
        timestamp = datetime.now().isoformat()
        data["timestamp"] = timestamp
        
        # Queue for async writing (non-blocking)
        data_queue.put(data)
        stats['queued'] += 1
        
        # Log less frequently to reduce overhead
        if stats['received'] % 10 == 0:
            logger.info(f"Received {stats['received']} packets | "
                       f"Queued: {data_queue.qsize()} | "
                       f"Written: {stats['written']}")
        
        # Fast response
        return jsonify({"status": "success"}), 200
        
    except Exception as e:
        logger.error(f"Error processing request: {e}")
        stats['errors'] += 1
        return jsonify({"error": str(e)}), 500

@app.route('/api/stats', methods=['GET'])
def get_stats():
    """Get server statistics."""
    return jsonify({
        "stats": stats,
        "queue_size": data_queue.qsize()
    }), 200

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
    
    # Start background writer thread
    writer_thread = threading.Thread(target=background_writer, daemon=True)
    writer_thread.start()
    logger.info("Background writer thread started")
    
    host_ip = get_ip_address()
    port = 5000
    
    print("\n" + "="*50)
    print(f" Sensor API Server Running (OPTIMIZED)")
    print(f" Address: http://{host_ip}:{port}")
    print(f" Endpoint: http://{host_ip}:{port}/api/sensor")
    print(f" Stats: http://{host_ip}:{port}/api/stats")
    print(f" Data directory: {DATA_DIR.absolute()}")
    print(f" Buffer size: {BUFFER_SIZE} records")
    print(f" Flush interval: {FLUSH_INTERVAL}s")
    print("="*50 + "\n")
    
    # Use threaded mode for better performance
    app.run(host='0.0.0.0', port=port, threaded=True)
