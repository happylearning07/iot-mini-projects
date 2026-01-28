#ifndef ENCODER_H_
#define ENCODER_H_

#include "sensor.h"
#include "subsystem.h"

/* flag macros */
#define FLAG_DELTA_MQ135 1 << 0
#define FLAG_DELTA_ANEMO 1 << 1
#define FLAG_DELTA_BTVOC 1 << 2
#define FLAG_DELTA_CO2EQ 1 << 3
#define FLAG_DELTA_STIAQ 1 << 4
#define FLAG_DELTA_IAQ 1 << 5
#define FLAG_DELTA_PRESR 1 << 6
#define FLAG_DELTA_HUMID 1 << 7
#define FLAG_DELTA_TEMPR 1 << 8
#define FLAG_PRESN_BTVOC 1 << 9
#define FLAG_PRESN_CO2EQ 1 << 10
#define FLAG_PRESN_STIAQ 1 << 11
#define FLAG_PRESN_BME680 1 << 12
#define FLAG_PRESN_MQ135 1 << 13
#define FLAG_PRESN_ANEMO 1 << 14
#define FLAG_PACKED_MQ135_ANEMO 1 << 15
#define FLAG_NEG_MQ135 1 << 16
#define FLAG_NEG_ANEMO 1 << 17
#define FLAG_NEG_BTVOC 1 << 18
#define FLAG_NEG_CO2EQ 1 << 19
#define FLAG_NEG_STIAQ 1 << 20
#define FLAG_NEG_IAQ 1 << 21
#define FLAG_NEG_PRESR 1 << 22
#define FLAG_NEG_HUMID 1 << 23
#define FLAG_NEG_TEMPR 1 << 24

typedef uint32_t flag_t; // flag interface

/* definition of the state structure of the encoder */
struct EncoderState {
  SensorData data;
  SensorData delta;
  uint16_t streak; // steps since the delta encoding started
};

#define ENCODER_OK 0x01
#define ENCODER_FAILURE 0x00

#define MAX_ENCODED_DATA_LEN 36

struct EncoderResult {
  uint8_t status;
  uint8_t data[MAX_ENCODED_DATA_LEN];
  flag_t flag;
  uint16_t streak;
  uint8_t len;
};

#define ENCODE_NO_BSEC_DATA 1 << 0
#define ENCODE_NO_MQ135_DATA 1 << 1
#define ENCODE_NO_ANEMO_DATA 1 << 2
#define ENCODE_NO_DELTA 1 << 3

class Encoder : public Subsystem {
private:
  static Encoder *instance;
  EncoderState state;
  EncoderResult encode_no_delta(SensorData new_state);

public:
  bool setup();
  void run(uint16_t dt);
  EncoderResult encode(SensorData new_state, uint8_t flags);
};

#endif // ENCODER_H_
