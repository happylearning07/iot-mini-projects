#include "decoder.h"
#include <string.h>

bool Decoder::setup() {
  memset(&state, 0, sizeof(state));
  return true;
}

void Decoder::run(uint16_t dt) { (void)dt; }

DecoderResult Decoder::decode_no_delta(const uint8_t *data, flag_t flags) {
  DecoderResult result;
  result.status = DECODER_OK;
  uint16_t idx = 0;

  int16_t temp = (data[idx] << 8) | data[idx + 1];
  idx += 2;
  if (flags & FLAG_NEG_TEMPR) {
    temp = -temp;
  }
  result.data.bsec_data.temperature = temp;

  result.data.bsec_data.humidity = (data[idx] << 8) | data[idx + 1];
  idx += 2;

  result.data.bsec_data.pressure =
      ((uint32_t)data[idx] << 24) | ((uint32_t)data[idx + 1] << 16) |
      ((uint32_t)data[idx + 2] << 8) | data[idx + 3];
  idx += 4;

  result.data.bsec_data.iaq = (data[idx] << 8) | data[idx + 1];
  idx += 2;

  result.data.bsec_data.iaqAccuracy = data[idx++];

  result.data.bsec_data.staticIaq = (data[idx] << 8) | data[idx + 1];
  idx += 2;

  result.data.bsec_data.co2Equivalent = (data[idx] << 8) | data[idx + 1];
  idx += 2;

  result.data.bsec_data.breathVoc = (data[idx] << 8) | data[idx + 1];
  idx += 2;

  result.data.bsec_data.gasPercentage = data[idx++];
  result.data.bsec_data.stabStatus = (data[idx] >> 4) & 0x0F;
  result.data.bsec_data.runInStatus = data[idx++] & 0x0F;

  result.data.mq135_data.analog =
      ((uint16_t)data[idx] << 4) | ((data[idx + 1] >> 4) & 0x0F);
  result.data.anemo_data =
      (((uint16_t)data[idx + 1] & 0x0F) << 8) | data[idx + 2];

  state.data = result.data;
  return result;
}

DecoderResult Decoder::decode(const EncoderResult &encoded) {
  DecoderResult result;
  result.status = DECODER_OK;

  if (encoded.flag & FLAG_PRESN_BME680) {
    return decode_no_delta(encoded.data, encoded.flag);
  }

  uint16_t idx = 0;
  flag_t flags = encoded.flag;
  SensorData delta;
  memset(&delta, 0, sizeof(delta));

  if (flags & FLAG_DELTA_TEMPR) {
    delta.bsec_data.temperature = encoded.data[idx++];
  } else {
    delta.bsec_data.temperature =
        (encoded.data[idx] << 8) | encoded.data[idx + 1];
    idx += 2;
  }
  if (flags & FLAG_NEG_TEMPR) {
    delta.bsec_data.temperature = -delta.bsec_data.temperature;
  }

  if (flags & FLAG_DELTA_HUMID) {
    delta.bsec_data.humidity = encoded.data[idx++];
  } else {
    delta.bsec_data.humidity = (encoded.data[idx] << 8) | encoded.data[idx + 1];
    idx += 2;
  }
  if (flags & FLAG_NEG_HUMID) {
    delta.bsec_data.humidity = -delta.bsec_data.humidity;
  }

  if (flags & FLAG_DELTA_PRESR) {
    delta.bsec_data.pressure = encoded.data[idx++];
  } else {
    delta.bsec_data.pressure = ((uint32_t)encoded.data[idx] << 24) |
                               ((uint32_t)encoded.data[idx + 1] << 16) |
                               ((uint32_t)encoded.data[idx + 2] << 8) |
                               encoded.data[idx + 3];
    idx += 4;
  }
  if (flags & FLAG_NEG_PRESR) {
    delta.bsec_data.pressure = -delta.bsec_data.pressure;
  }

  if (flags & FLAG_DELTA_IAQ) {
    delta.bsec_data.iaq = encoded.data[idx++];
  } else {
    delta.bsec_data.iaq = (encoded.data[idx] << 8) | encoded.data[idx + 1];
    idx += 2;
  }
  if (flags & FLAG_NEG_IAQ) {
    delta.bsec_data.iaq = -delta.bsec_data.iaq;
  }

  delta.bsec_data.iaqAccuracy = encoded.data[idx++];

  if (flags & FLAG_DELTA_STIAQ) {
    delta.bsec_data.staticIaq = encoded.data[idx++];
  } else {
    delta.bsec_data.staticIaq =
        (encoded.data[idx] << 8) | encoded.data[idx + 1];
    idx += 2;
  }
  if (flags & FLAG_NEG_STIAQ) {
    delta.bsec_data.staticIaq = -delta.bsec_data.staticIaq;
  }

  if (flags & FLAG_DELTA_CO2EQ) {
    delta.bsec_data.co2Equivalent = encoded.data[idx++];
  } else {
    delta.bsec_data.co2Equivalent =
        (encoded.data[idx] << 8) | encoded.data[idx + 1];
    idx += 2;
  }
  if (flags & FLAG_NEG_CO2EQ) {
    delta.bsec_data.co2Equivalent = -delta.bsec_data.co2Equivalent;
  }

  if (flags & FLAG_DELTA_BTVOC) {
    delta.bsec_data.breathVoc = encoded.data[idx++];
  } else {
    delta.bsec_data.breathVoc =
        (encoded.data[idx] << 8) | encoded.data[idx + 1];
    idx += 2;
  }
  if (flags & FLAG_NEG_BTVOC) {
    delta.bsec_data.breathVoc = -delta.bsec_data.breathVoc;
  }

  delta.bsec_data.gasPercentage = encoded.data[idx++];
  delta.bsec_data.stabStatus = (encoded.data[idx] >> 4) & 0x0F;
  delta.bsec_data.runInStatus = encoded.data[idx++] & 0x0F;

  if ((flags & FLAG_DELTA_MQ135) && (flags & FLAG_DELTA_ANEMO)) {
    delta.mq135_data.analog = (encoded.data[idx] >> 4) & 0x0F;
    delta.anemo_data = encoded.data[idx++] & 0x0F;
  } else {
    delta.mq135_data.analog = ((uint16_t)encoded.data[idx] << 4) |
                              ((encoded.data[idx + 1] >> 4) & 0x0F);
    delta.anemo_data =
        (((uint16_t)encoded.data[idx + 1] & 0x0F) << 8) | encoded.data[idx + 2];
    idx += 3;
  }
  if (flags & FLAG_NEG_MQ135) {
    delta.mq135_data.analog = -delta.mq135_data.analog;
  }
  if (flags & FLAG_NEG_ANEMO) {
    delta.anemo_data = -delta.anemo_data;
  }

  result.data.bsec_data.temperature =
      state.data.bsec_data.temperature + delta.bsec_data.temperature;
  result.data.bsec_data.humidity =
      state.data.bsec_data.humidity + delta.bsec_data.humidity;
  result.data.bsec_data.pressure =
      state.data.bsec_data.pressure + delta.bsec_data.pressure;
  result.data.bsec_data.iaq = state.data.bsec_data.iaq + delta.bsec_data.iaq;
  result.data.bsec_data.iaqAccuracy = delta.bsec_data.iaqAccuracy;
  result.data.bsec_data.staticIaq =
      state.data.bsec_data.staticIaq + delta.bsec_data.staticIaq;
  result.data.bsec_data.co2Equivalent =
      state.data.bsec_data.co2Equivalent + delta.bsec_data.co2Equivalent;
  result.data.bsec_data.breathVoc =
      state.data.bsec_data.breathVoc + delta.bsec_data.breathVoc;
  result.data.bsec_data.gasPercentage = delta.bsec_data.gasPercentage;
  result.data.bsec_data.stabStatus = delta.bsec_data.stabStatus;
  result.data.bsec_data.runInStatus = delta.bsec_data.runInStatus;

  result.data.mq135_data.analog =
      state.data.mq135_data.analog + delta.mq135_data.analog;
  result.data.anemo_data = state.data.anemo_data + delta.anemo_data;

  state.data = result.data;
  return result;
}
