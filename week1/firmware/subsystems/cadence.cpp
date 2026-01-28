#include "cadence.h"

bool Cadence::setup() {
  sensor_interval = DEFAULT_SENSOR_INTERVAL_MS;
  transmission_interval = DEFAULT_TRANSMISSION_INTERVAL_MS;
  sensor_accumulator = 0;
  transmission_accumulator = 0;
  return true;
}

void Cadence::run(uint16_t dt) {
  sensor_accumulator += dt;
  transmission_accumulator += dt;
}

void Cadence::setSensorInterval(uint16_t ms) { sensor_interval = ms; }

void Cadence::setTransmissionInterval(uint16_t ms) {
  transmission_interval = ms;
}

bool Cadence::shouldUpdateSensor() {
  if (sensor_accumulator >= sensor_interval) {
    sensor_accumulator -= sensor_interval;
    return true;
  }
  return false;
}

bool Cadence::shouldTransmit() {
  if (transmission_accumulator >= transmission_interval) {
    transmission_accumulator -= transmission_interval;
    return true;
  }
  return false;
}

void Cadence::reset() {
  sensor_accumulator = 0;
  transmission_accumulator = 0;
}
