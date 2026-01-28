#!/bin/bash

chmod a+rw /dev/ttyUSB0
arduino-cli compile --fqbn Heltec-esp32:esp32:heltec_wifi_lora_32_V3 && arduino-cli upload -p /dev/ttyUSB0 --fqbn Heltec-esp32:esp32:heltec_wifi_lora_32_V3 && arduino-cli monitor -p /dev/ttyUSB0 --config 115200
