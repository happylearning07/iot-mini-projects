#!/usr/bin/env python3
"""
LoRa Environmental Sensor Data Logger

Reads JSON data from serial port and stores with incremental persistence.
Data is saved to both CSV (for analysis) and JSON (for backup).

Usage:
    python data_logger.py [--port /dev/ttyUSB0] [--baud 115200] [--output data]
"""

import argparse
import json
import os
import signal
import sys
import time
from datetime import datetime
from pathlib import Path

try:
    import serial
except ImportError:
    print("Error: pyserial not installed. Run: pip install pyserial")
    sys.exit(1)


class DataLogger:
    """Handles serial reading and persistent data storage."""

    def __init__(self, port: str, baud: int, output_dir: str):
        self.port = port
        self.baud = baud
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

        # File paths
        self.csv_path = self.output_dir / "sensor_data.csv"
        self.jsonl_path = self.output_dir / "sensor_data.jsonl"
        self.checkpoint_path = self.output_dir / ".checkpoint.json"

        # State
        self.serial_conn = None
        self.running = False
        self.records_count = 0
        self.last_save_time = time.time()
        self.buffer = []

        # Load checkpoint if exists
        self.load_checkpoint()

    def load_checkpoint(self):
        """Load progress from checkpoint file."""
        if self.checkpoint_path.exists():
            try:
                with open(self.checkpoint_path, "r") as f:
                    checkpoint = json.load(f)
                    self.records_count = checkpoint.get("records_count", 0)
                    print(f"Resuming from checkpoint: {self.records_count} records")
            except Exception as e:
                print(f"Warning: Could not load checkpoint: {e}")

    def save_checkpoint(self):
        """Save progress to checkpoint file."""
        checkpoint = {
            "records_count": self.records_count,
            "last_save": datetime.now().isoformat(),
        }
        with open(self.checkpoint_path, "w") as f:
            json.dump(checkpoint, f)

    def init_csv(self):
        """Initialize CSV file with headers if it doesn't exist."""
        if not self.csv_path.exists():
            headers = [
                "timestamp",
                "device_id",
                "sequence",
                "uptime",
                "temperature",
                "humidity",
                "pressure",
                "iaq",
                "iaq_accuracy",
                "iaq_label",
                "static_iaq",
                "co2_ppm",
                "voc_ppm",
                "gas_percent",
                "stabilized",
                "run_in_complete",
                "rssi",
                "snr",
            ]
            with open(self.csv_path, "w") as f:
                f.write(",".join(headers) + "\n")
            print(f"Created CSV: {self.csv_path}")

    def append_record(self, data: dict):
        """Append a data record to storage."""
        timestamp = datetime.now().isoformat()
        data["timestamp"] = timestamp

        # Append to JSONL (one JSON object per line)
        with open(self.jsonl_path, "a") as f:
            f.write(json.dumps(data) + "\n")

        # Append to CSV
        row = [
            timestamp,
            str(data.get("device_id", "")),
            str(data.get("sequence", "")),
            str(data.get("uptime", "")),
            str(data.get("temperature", "")),
            str(data.get("humidity", "")),
            str(data.get("pressure", "")),
            str(data.get("iaq", "")),
            str(data.get("iaq_accuracy", "")),
            data.get("iaq_label", ""),
            str(data.get("static_iaq", "")),
            str(data.get("co2_ppm", "")),
            str(data.get("voc_ppm", "")),
            str(data.get("gas_percent", "")),
            str(data.get("stabilized", "")),
            str(data.get("run_in_complete", "")),
            str(data.get("rssi", "")),
            str(data.get("snr", "")),
        ]
        with open(self.csv_path, "a") as f:
            f.write(",".join(row) + "\n")

        self.records_count += 1

        # Incremental save checkpoint every 10 records or 30 seconds
        if self.records_count % 10 == 0 or (time.time() - self.last_save_time) > 30:
            self.save_checkpoint()
            self.last_save_time = time.time()

    def connect(self):
        """Connect to serial port."""
        try:
            self.serial_conn = serial.Serial(self.port, self.baud, timeout=1)
            print(f"Connected to {self.port} at {self.baud} baud")
            return True
        except serial.SerialException as e:
            print(f"Error: Could not open {self.port}: {e}")
            return False

    def process_line(self, line: str):
        """Process a single line from serial."""
        line = line.strip()
        if not line:
            return

        try:
            data = json.loads(line)
            msg_type = data.get("type", "unknown")

            if msg_type == "data":
                self.append_record(data)
                # Print summary
                print(
                    f"[{self.records_count}] "
                    f"T={data.get('temperature', 0):.1f}°C "
                    f"H={data.get('humidity', 0):.1f}% "
                    f"IAQ={data.get('iaq', 0)} "
                    f"CO2={data.get('co2_ppm', 0)}ppm "
                    f"RSSI={data.get('rssi', 0)}"
                )

            elif msg_type == "error":
                print(f"[ERROR] {data.get('reason', 'unknown')}")

            elif msg_type == "init":
                print(f"[INIT] Device: {data.get('device', 'unknown')}")

            elif msg_type == "ready":
                print(f"[READY] Status: {data.get('status', 'unknown')}")

            else:
                print(f"[{msg_type.upper()}] {data}")

        except json.JSONDecodeError:
            # Not JSON, print as debug
            if line.startswith("{"):
                print(f"[PARSE ERROR] {line[:50]}...")
            else:
                print(f"[DEBUG] {line}")

    def run(self):
        """Main loop: read from serial and process."""
        if not self.connect():
            return

        self.init_csv()
        self.running = True

        print(f"\nLogging to: {self.output_dir}")
        print(f"CSV file: {self.csv_path}")
        print(f"JSONL file: {self.jsonl_path}")
        print("Press Ctrl+C to stop\n")

        try:
            while self.running:
                if self.serial_conn.in_waiting:
                    try:
                        line = self.serial_conn.readline().decode("utf-8", errors="ignore")
                        self.process_line(line)
                    except Exception as e:
                        print(f"[READ ERROR] {e}")
                else:
                    time.sleep(0.01)

        except KeyboardInterrupt:
            print("\n\nStopping...")

        finally:
            self.shutdown()

    def shutdown(self):
        """Clean shutdown."""
        self.running = False
        self.save_checkpoint()
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
        print(f"Saved {self.records_count} records")
        print(f"Data stored in: {self.output_dir}")


def main():
    parser = argparse.ArgumentParser(description="LoRa Sensor Data Logger")
    parser.add_argument(
        "--port", "-p", default="/dev/ttyUSB0", help="Serial port (default: /dev/ttyUSB0)"
    )
    parser.add_argument(
        "--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)"
    )
    parser.add_argument(
        "--output", "-o", default="data", help="Output directory (default: data)"
    )
    args = parser.parse_args()

    logger = DataLogger(args.port, args.baud, args.output)

    # Handle SIGTERM gracefully
    signal.signal(signal.SIGTERM, lambda s, f: logger.shutdown())

    logger.run()


if __name__ == "__main__":
    main()
