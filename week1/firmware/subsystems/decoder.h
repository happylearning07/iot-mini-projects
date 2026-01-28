#ifndef DECODER_H_
#define DECODER_H_

#include "encoder.h"
#include "subsystem.h"

struct DecoderState {
  SensorData data;
};

#define DECODER_OK 0x01
#define DECODER_FAILURE 0x00

struct DecoderResult {
  uint8_t status;
  SensorData data;
};

class Decoder : public Subsystem {
private:
  DecoderState state;
  DecoderResult decode_no_delta(const uint8_t *encoded_data, flag_t flags);

public:
  bool setup();
  void run(uint16_t dt);
  DecoderResult decode(const EncoderResult &result);
};

#endif // DECODER_H_
