#include "framing.h"
#include <string.h>

#ifndef DEVICE_ID
#define DEVICE_ID 0x01
#endif

static uint16_t calculate_crc16(const uint8_t *data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc = crc >> 1;
      }
    }
  }
  return crc;
}

bool Framing::setup() { return true; }

void Framing::run(uint16_t dt) { (void)dt; }

FrameHeader Framing::frame(EncoderResult &result, uint16_t sequence,
                           FrameBuffer_t &buffer, uint16_t &crc) {
  FrameHeader header;
  header.sof = SOF;
  header.deviceid = DEVICE_ID;
  header.flags = result.flag;
  header.sequence = sequence;
  header.len = result.len;

  uint16_t idx = 0;
  buffer[idx++] = header.sof;
  buffer[idx++] = header.deviceid;

  buffer[idx++] = (header.flags >> 24) & 0xFF;
  buffer[idx++] = (header.flags >> 16) & 0xFF;
  buffer[idx++] = (header.flags >> 8) & 0xFF;
  buffer[idx++] = header.flags & 0xFF;

  buffer[idx++] = (header.sequence >> 8) & 0xFF;
  buffer[idx++] = header.sequence & 0xFF;

  buffer[idx++] = (header.len >> 8) & 0xFF;
  buffer[idx++] = header.len & 0xFF;

  memcpy(&buffer[idx], result.data, result.len);
  idx += result.len;

  crc = calculate_crc16(buffer, idx);
  buffer[idx++] = (crc >> 8) & 0xFF;
  buffer[idx++] = crc & 0xFF;

  uint16_t final_len = escape(buffer, idx);

  header.len = final_len;
  return header;
}

uint16_t Framing::escape(FrameBuffer_t &buffer, uint16_t len) {
  uint8_t temp[MAX_FRAME_LEN];
  uint16_t write_idx = 0;

  temp[write_idx++] = buffer[0];

  for (uint16_t read_idx = 1; read_idx < len; read_idx++) {
    if (buffer[read_idx] == SOF || buffer[read_idx] == ESC) {
      temp[write_idx++] = ESC;
      temp[write_idx++] = buffer[read_idx];
    } else {
      temp[write_idx++] = buffer[read_idx];
    }
  }

  memcpy(buffer, temp, write_idx);
  return write_idx;
}
