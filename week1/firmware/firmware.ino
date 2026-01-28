#include <Arduino.h>

#include "subsystems/cadence.h"
#include "subsystems/encoder.h"
#include "subsystems/framing.h"
#include "subsystems/queue.h"
#include "subsystems/sensor.h"
#include "subsystems/transmission.h"

#include "subsystems/cadence.cpp"
#include "subsystems/encoder.cpp"
#include "subsystems/framing.cpp"
#include "subsystems/queue.cpp"
#include "subsystems/sensor.cpp"
#include "subsystems/subsystem.cpp"
#include "subsystems/transmission.cpp"

#define BAUD 115200

Cadence cadence;
Sensor sensor;
Encoder encoder;
Queue queue;
Framing framing;
Transmission transmission;

uint16_t sequence = 0;
uint32_t last_millis = 0;
bool sensor_ok = false;

void setup() {
  Serial.begin(BAUD);
  Serial.println("IoT Firmware Starting...");

  if (!cadence.setup()) {
    Serial.println("ERROR: Cadence setup failed");
  }

  sensor_ok = sensor.setup();
  if (!sensor_ok) {
    Serial.println(
        "WARNING: Sensor setup failed - will run without BME680 sensor");
  }

  if (!encoder.setup()) {
    Serial.println("ERROR: Encoder setup failed");
  }
  if (!queue.setup()) {
    Serial.println("ERROR: Queue setup failed");
  }
  if (!framing.setup()) {
    Serial.println("ERROR: Framing setup failed");
  }
  if (!transmission.setup()) {
    Serial.println("ERROR: Transmission setup failed");
  }

  cadence.setSensorInterval(1000);
  cadence.setTransmissionInterval(10000);

  last_millis = millis();
  Serial.println("Initialization complete");
  Serial.printf("Sensor interval: 1000ms, Transmission interval: 10000ms\n");
  if (!sensor_ok) {
    Serial.println("NOTE: Running in degraded mode without BSEC sensor");
  }
}

void loop() {
  uint32_t current_millis = millis();
  uint16_t dt = (uint16_t)(current_millis - last_millis);
  last_millis = current_millis;

  cadence.run(dt);

  if (cadence.shouldUpdateSensor() && sensor_ok) {
    sensor.run(dt);

    if (sensor.has_bsec_error()) {
      Serial.println("WARNING: BSEC error detected");
    }

    if (sensor.has_new_bsec_data()) {
      SensorData data = sensor.get_data();

      uint8_t encode_flags = 0;
      EncoderResult result = encoder.encode(data, encode_flags);

      if (result.status == ENCODER_OK) {
        queue.push(result);
        Serial.printf("Encoded data (len=%d, streak=%d, queue=%d/%d)\n",
                      result.len, result.streak, queue.size(),
                      queue.capacity());
      }
    }
  }

  if (cadence.shouldTransmit()) {
    if (!queue.isEmpty()) {
      EncoderResult to_transmit;
      queue.pop(to_transmit);

      FrameBuffer_t frame;
      uint16_t crc;
      FrameHeader header = framing.frame(to_transmit, sequence++, frame, crc);

      Serial.printf("Transmitting frame (seq=%d, len=%d, crc=0x%04X)\n",
                    sequence - 1, header.len, crc);

      transmission.transmit(frame, header.len);
    } else {
      if (sensor_ok) {
        Serial.println("No data to transmit");
      }
    }
  }

  transmission.run(dt);
  delay(10);
}
