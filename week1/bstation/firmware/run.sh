#!/bin/bash

chmod a+rw /dev/ttyUSB1
arduino-cli compile --fqbn Heltec-esp32:esp32:heltec_wifi_lora_32_V3 && arduino-cli upload -p /dev/ttyUSB1 --fqbn Heltec-esp32:esp32:heltec_wifi_lora_32_V3 && arduino-cli monitor -p /dev/ttyUSB1 --config 115200
