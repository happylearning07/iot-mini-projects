#include "packet.h"

// CRC-16/CCITT polynomial (0x1021) - commonly used in LoRa/LoRaWAN
static const uint16_t CRC16_POLY = 0x1021;
static const uint16_t CRC16_INIT = 0xFFFF;

uint16_t calculateCRC16(const uint8_t *data, uint8_t length) {
  uint16_t crc = CRC16_INIT;

  for (uint8_t i = 0; i < length; i++) {
    crc ^= ((uint16_t)data[i] << 8);
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ CRC16_POLY;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

void initPacket(LoRaPacket *pkt, uint16_t deviceId) {
  pkt->version = PACKET_VERSION;
  pkt->deviceId = deviceId;
  pkt->sequence = 0;
  pkt->uptime = 0;
  pkt->temperature = 0;
  pkt->humidity = 0;
  pkt->pressure = 0;
  pkt->iaq = 0;
  pkt->iaqAccuracy = 0;
  pkt->staticIaq = 0;
  pkt->co2Equivalent = 0;
  pkt->breathVoc = 0;
  pkt->gasPercentage = 0;
  pkt->stabStatus = 0;
  pkt->runInStatus = 0;
  pkt->crc = 0;
}

void populatePacket(LoRaPacket *pkt, uint16_t sequence, uint32_t uptimeSec,
                    int16_t temperature, uint16_t humidity, uint32_t pressure,
                    uint16_t iaq, uint8_t iaqAccuracy, uint16_t staticIaq,
                    uint16_t co2Equivalent, uint16_t breathVoc,
                    uint8_t gasPercentage, uint8_t stabStatus,
                    uint8_t runInStatus) {
  pkt->sequence = sequence;
  pkt->uptime = uptimeSec;
  pkt->temperature = temperature;
  pkt->humidity = humidity;
  pkt->pressure = pressure & 0xFFFFFF;
  pkt->iaq = iaq;
  pkt->iaqAccuracy = iaqAccuracy;
  pkt->staticIaq = staticIaq;
  pkt->co2Equivalent = co2Equivalent;
  pkt->breathVoc = breathVoc;
  pkt->gasPercentage = gasPercentage;
  pkt->stabStatus = stabStatus;
  pkt->runInStatus = runInStatus;
}

uint8_t encodePacket(LoRaPacket &pkt, uint8_t *buffer) {
  uint8_t idx = 0;

  buffer[idx++] = pkt.version;
  buffer[idx++] = (pkt.deviceId >> 8) & 0xFF;
  buffer[idx++] = pkt.deviceId & 0xFF;
  buffer[idx++] = (pkt.sequence >> 8) & 0xFF;
  buffer[idx++] = pkt.sequence & 0xFF;
  buffer[idx++] = (pkt.uptime >> 24) & 0xFF;
  buffer[idx++] = (pkt.uptime >> 16) & 0xFF;
  buffer[idx++] = (pkt.uptime >> 8) & 0xFF;
  buffer[idx++] = pkt.uptime & 0xFF;
  buffer[idx++] = (pkt.temperature >> 8) & 0xFF;
  buffer[idx++] = pkt.temperature & 0xFF;
  buffer[idx++] = (pkt.humidity >> 8) & 0xFF;
  buffer[idx++] = pkt.humidity & 0xFF;
  buffer[idx++] = (pkt.pressure >> 16) & 0xFF;
  buffer[idx++] = (pkt.pressure >> 8) & 0xFF;
  buffer[idx++] = pkt.pressure & 0xFF;
  buffer[idx++] = (pkt.iaq >> 8) & 0xFF;
  buffer[idx++] = pkt.iaq & 0xFF;
  buffer[idx++] = pkt.iaqAccuracy;
  buffer[idx++] = (pkt.staticIaq >> 8) & 0xFF;
  buffer[idx++] = pkt.staticIaq & 0xFF;
  buffer[idx++] = (pkt.co2Equivalent >> 8) & 0xFF;
  buffer[idx++] = pkt.co2Equivalent & 0xFF;
  buffer[idx++] = (pkt.breathVoc >> 8) & 0xFF;
  buffer[idx++] = pkt.breathVoc & 0xFF;
  buffer[idx++] = pkt.gasPercentage;
  buffer[idx++] = pkt.stabStatus;
  buffer[idx++] = pkt.runInStatus;

  pkt.crc = calculateCRC16(buffer, idx);
  buffer[idx++] = (pkt.crc >> 8) & 0xFF;
  buffer[idx++] = pkt.crc & 0xFF;

  return idx;
}

bool decodePacket(const uint8_t *buffer, uint8_t size, LoRaPacket &pkt) {
  if (size < PACKET_SIZE) {
    return false;
  }

  uint8_t idx = 0;
  pkt.version = buffer[idx++];

  if (pkt.version != PACKET_VERSION) {
    return false;
  }

  pkt.deviceId = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];
  idx += 2;
  pkt.sequence = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];
  idx += 2;
  pkt.uptime = ((uint32_t)buffer[idx] << 24) |
               ((uint32_t)buffer[idx + 1] << 16) |
               ((uint32_t)buffer[idx + 2] << 8) | buffer[idx + 3];
  idx += 4;
  pkt.temperature = (int16_t)(((uint16_t)buffer[idx] << 8) | buffer[idx + 1]);
  idx += 2;
  pkt.humidity = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];
  idx += 2;
  pkt.pressure = ((uint32_t)buffer[idx] << 16) |
                 ((uint32_t)buffer[idx + 1] << 8) | buffer[idx + 2];
  idx += 3;
  pkt.iaq = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];
  idx += 2;
  pkt.iaqAccuracy = buffer[idx++];
  pkt.staticIaq = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];
  idx += 2;
  pkt.co2Equivalent = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];
  idx += 2;
  pkt.breathVoc = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];
  idx += 2;
  pkt.gasPercentage = buffer[idx++];
  pkt.stabStatus = buffer[idx++];
  pkt.runInStatus = buffer[idx++];
  pkt.crc = ((uint16_t)buffer[idx] << 8) | buffer[idx + 1];

  uint16_t calculatedCrc = calculateCRC16(buffer, PACKET_SIZE - 2);
  if (calculatedCrc != pkt.crc) {
    return false;
  }

  return true;
}

void printPacket(const LoRaPacket &pkt) {
  Serial.println("=== LoRa Packet (BSEC2) ===");
  Serial.printf("  Version:     0x%02X\n", pkt.version);
  Serial.printf("  Device ID:   0x%04X (%u)\n", pkt.deviceId, pkt.deviceId);
  Serial.printf("  Sequence:    %u\n", pkt.sequence);
  Serial.printf("  Uptime:      %u sec (%02d:%02d:%02d)\n", pkt.uptime,
                pkt.uptime / 3600, (pkt.uptime % 3600) / 60, pkt.uptime % 60);

  Serial.println("--- Environmental Data ---");
  Serial.printf("  Temperature: %d.%02d C\n", pkt.temperature / 100,
                abs(pkt.temperature % 100));
  Serial.printf("  Humidity:    %u.%02u %%\n", pkt.humidity / 100,
                pkt.humidity % 100);
  Serial.printf("  Pressure:    %u.%03u MPa\n", pkt.pressure / 1000,
                pkt.pressure % 1000);

  Serial.println("--- IAQ Data ---");
  Serial.printf("  IAQ:         %u (accuracy: %u)\n", pkt.iaq, pkt.iaqAccuracy);
  Serial.printf("  Static IAQ:  %u\n", pkt.staticIaq);
  Serial.printf("  CO2 equiv:   %u ppm\n", pkt.co2Equivalent);
  Serial.printf("  bVOC equiv:  %u.%02u ppm\n", pkt.breathVoc / 100,
                pkt.breathVoc % 100);
  Serial.printf("  Gas:         %u%%\n", pkt.gasPercentage);

  Serial.println("--- Status ---");
  Serial.printf("  Stabilized:  %s\n", pkt.stabStatus ? "Yes" : "No");
  Serial.printf("  Run-in:      %s\n",
                pkt.runInStatus ? "Complete" : "Ongoing");
  Serial.printf("  CRC:         0x%04X\n", pkt.crc);
  Serial.println("============================");
}
