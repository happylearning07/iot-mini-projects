#ifndef FRAMING_H_
#define FRAMING_H_

#include "encoder.h"
#include "subsystem.h"

#define SOF 0x7E
#define ESC 0x7F

#define MAX_FRAME_LEN 72 // assuming every byte is escaped

struct FrameHeader {
  uint8_t sof;
  uint8_t deviceid;
  uint32_t flags;
  uint16_t sequence;
  uint16_t len;
  // variable length payload
  // 16 bit crc
};

typedef uint8_t FrameBuffer_t[MAX_FRAME_LEN];

class Framing : public Subsystem {
public:
  bool setup();
  void run(uint16_t dt);
  FrameHeader frame(EncoderResult &result, uint16_t sequence,
                    FrameBuffer_t &buffer, uint16_t &crc);
  uint16_t escape(FrameBuffer_t &buffer,
                  uint16_t len); // escape the escape byte and the sof byte (
                                 // returns the final len)
};

#endif // FRAMING_H_
