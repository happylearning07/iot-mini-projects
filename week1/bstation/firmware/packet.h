/**
 * @file packet.h
 * @brief LoRaWAN packet encoding/decoding for BSEC2 environmental sensor data
 * All multi-byte fields are Big Endian (network byte order)
 */

#ifndef PACKET_H_
#define PACKET_H_

#include <Arduino.h>

#define PACKET_VERSION 0x02
#define PACKET_SIZE 30       // Total packet size in bytes
#define PACKET_HEADER_SIZE 5 // Version + DeviceID + Sequence

/**
 * @brief Structure holding BSEC2 sensor data for transmission
 *
 * Uses heat-compensated temperature/humidity from BSEC instead of raw values.
 * Includes IAQ (Indoor Air Quality) metrics and derived gas outputs.
 */
struct LoRaPacket {
  uint8_t version;
  uint16_t deviceId;
  uint16_t sequence;
  uint32_t uptime;

  // BSEC Compensated Environmental Data
  int16_t temperature;
  uint16_t humidity;
  uint32_t pressure;

  // BSEC IAQ Outputs
  uint16_t iaq;
  uint8_t iaqAccuracy;
  uint16_t staticIaq;
  uint16_t co2Equivalent;
  uint16_t breathVoc;
  uint8_t gasPercentage;

  // BSEC Status
  uint8_t stabStatus;
  uint8_t runInStatus;

  uint16_t crc;
};

/**
 * @brief Initialize a packet with default values
 * @param pkt Pointer to packet structure
 * @param deviceId Device identifier to use
 */
void initPacket(LoRaPacket *pkt, uint16_t deviceId);

/**
 * @brief Populate packet with BSEC2 sensor readings
 */
void populatePacket(LoRaPacket *pkt, uint16_t sequence, uint32_t uptimeSec,
                    int16_t temperature, uint16_t humidity, uint32_t pressure,
                    uint16_t iaq, uint8_t iaqAccuracy, uint16_t staticIaq,
                    uint16_t co2Equivalent, uint16_t breathVoc,
                    uint8_t gasPercentage, uint8_t stabStatus,
                    uint8_t runInStatus);

/**
 * @brief Encode packet to byte buffer for transmission
 */
uint8_t encodePacket(LoRaPacket &pkt, uint8_t *buffer);

/**
 * @brief Decode byte buffer to packet structure
 */
bool decodePacket(const uint8_t *buffer, uint8_t size, LoRaPacket &pkt);

/**
 * @brief Calculate CRC-16/CCITT checksum
 */
uint16_t calculateCRC16(const uint8_t *data, uint8_t length);

/**
 * @brief Print packet contents to Serial for debugging
 */
void printPacket(const LoRaPacket &pkt);

#endif // PACKET_H_
