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
 * @param pkt Pointer to packet structure
 * @param sequence Current sequence number
 * @param uptimeSec Uptime in seconds
 * @param temperature Compensated temperature (°C × 100)
 * @param humidity Compensated humidity (% × 100)
 * @param pressure Pressure in Pa
 * @param iaq IAQ index (0-500)
 * @param iaqAccuracy IAQ accuracy (0-3)
 * @param staticIaq Static IAQ index
 * @param co2Equivalent CO2 equivalent in ppm
 * @param breathVoc Breath VOC equivalent (ppm × 100)
 * @param gasPercentage Gas percentage (0-100)
 * @param stabStatus Stabilization status
 * @param runInStatus Run-in status
 */
void populatePacket(LoRaPacket *pkt, uint16_t sequence, uint32_t uptimeSec,
                    int16_t temperature, uint16_t humidity, uint32_t pressure,
                    uint16_t iaq, uint8_t iaqAccuracy, uint16_t staticIaq,
                    uint16_t co2Equivalent, uint16_t breathVoc,
                    uint8_t gasPercentage, uint8_t stabStatus,
                    uint8_t runInStatus);

/**
 * @brief Encode packet to byte buffer for transmission
 * @param pkt Packet structure to encode
 * @param buffer Output buffer (must be at least PACKET_SIZE bytes)
 * @return Number of bytes written (PACKET_SIZE on success)
 */
uint8_t encodePacket(LoRaPacket &pkt, uint8_t *buffer);

/**
 * @brief Decode byte buffer to packet structure
 * @param buffer Input buffer containing encoded packet
 * @param size Size of input buffer
 * @param pkt Output packet structure
 * @return true if decode successful and CRC valid, false otherwise
 */
bool decodePacket(const uint8_t *buffer, uint8_t size, LoRaPacket &pkt);

/**
 * @brief Calculate CRC-16/CCITT checksum
 * @param data Data buffer
 * @param length Length of data
 * @return 16-bit CRC value
 */
uint16_t calculateCRC16(const uint8_t *data, uint8_t length);

/**
 * @brief Print packet contents to Serial for debugging
 * @param pkt Packet to print
 */
void printPacket(const LoRaPacket &pkt);

#endif // PACKET_H_
