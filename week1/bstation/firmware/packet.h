/**
 * @file packet.h
 * @brief LoRaWAN packet encoding/decoding for BSEC2 environmental sensor data
 * All multi-byte fields are Big Endian (network byte order)
 */

#ifndef PACKET_H_
#define PACKET_H_

#include <Arduino.h>

#define PACKET_VERSION 0x02
#define PACKET_SIZE 31        // Total packet size in bytes (ENV)
#define ANALOG_PACKET_SIZE 16 // Total packet size in bytes (ANALOG)
#define PACKET_HEADER_SIZE 6  // Version + Type + DeviceID + Sequence

// Packet Types
#define PACKET_TYPE_ENV 0x01
#define PACKET_TYPE_ANALOG 0x02

/**
 * @brief Structure holding BSEC2 sensor data for transmission
 *
 * Uses heat-compensated temperature/humidity from BSEC instead of raw values.
 * Includes IAQ (Indoor Air Quality) metrics and derived gas outputs.
 */
struct LoRaPacket {
  uint8_t version;
  uint8_t packetType; // PACKET_TYPE_ENV
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
 * @brief Structure holding analog sensor data
 * Sent frequently (every 10ms)
 */
struct AnalogPacket {
  uint8_t version;
  uint8_t packetType; // PACKET_TYPE_ANALOG
  uint16_t deviceId;
  uint16_t sequence;
  uint32_t uptime;
  uint16_t mq135;
  uint16_t anemometer;
  uint16_t crc;
};

/**
 * @brief Initialize a packet with default values
 * @param pkt Pointer to packet structure
 * @param deviceId Device identifier to use
 */
void initPacket(LoRaPacket *pkt, uint16_t deviceId);

/**
 * @brief Initialize an analog packet with default values
 * @param pkt Pointer to packet structure
 * @param deviceId Device identifier to use
 */
void initAnalogPacket(AnalogPacket *pkt, uint16_t deviceId);

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
 * @brief Populate analog packet with sensor readings
 * @param pkt Pointer to packet structure
 * @param sequence Current sequence number
 * @param uptimeSec Uptime in seconds
 * @param mq135 Raw MQ135 reading
 * @param anemometer Raw anemometer reading
 */
void populateAnalogPacket(AnalogPacket *pkt, uint16_t sequence,
                          uint32_t uptimeSec, uint16_t mq135,
                          uint16_t anemometer);

/**
 * @brief Encode packet to byte buffer for transmission
 * @param pkt Packet structure to encode
 * @param buffer Output buffer (must be at least PACKET_SIZE bytes)
 * @return Number of bytes written (PACKET_SIZE on success)
 */
uint8_t encodePacket(LoRaPacket &pkt, uint8_t *buffer);

/**
 * @brief Encode analog packet to byte buffer for transmission
 * @param pkt Packet structure to encode
 * @param buffer Output buffer (must be at least ANALOG_PACKET_SIZE bytes)
 * @return Number of bytes written (ANALOG_PACKET_SIZE on success)
 */
uint8_t encodeAnalogPacket(AnalogPacket &pkt, uint8_t *buffer);

/**
 * @brief Decode byte buffer to packet structure
 * @param buffer Input buffer containing encoded packet
 * @param size Size of input buffer
 * @param pkt Output packet structure
 * @return true if decode successful and CRC valid, false otherwise
 */
bool decodePacket(const uint8_t *buffer, uint8_t size, LoRaPacket &pkt);

/**
 * @brief Decode byte buffer to analog packet structure
 * @param buffer Input buffer containing encoded packet
 * @param size Size of input buffer
 * @param pkt Output packet structure
 * @return true if decode successful and CRC valid, false otherwise
 */
bool decodeAnalogPacket(const uint8_t *buffer, uint8_t size, AnalogPacket &pkt);

/**
 * @brief Calculate CRC-16/CCITT checksum
 * @param data Data buffer
 * @param length Length of data
 * @return 16-bit CRC value
 */
uint16_t bsecCRC16(const uint8_t *data, uint8_t length);

/**
 * @brief Print packet contents to Serial for debugging
 * @param pkt Packet to print
 */
void printPacket(const LoRaPacket &pkt);

/**
 * @brief Print analog packet contents to Serial for debugging
 * @param pkt Packet to print
 */
void printAnalogPacket(const AnalogPacket &pkt);

#endif // PACKET_H_
