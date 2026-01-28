#include "encoder.h"

bool Encoder::setup() { return true; }

void Encoder::run(uint16_t dt) { return; }

EncoderResult Encoder::encode_no_delta(SensorData new_data) {
  EncoderResult result;
  flag_t flag = FLAG_PRESN_BME680 | FLAG_PRESN_MQ135 | FLAG_PRESN_ANEMO;
  uint8_t byte_index = 0;

  int16_t temp = new_data.bsec_data.temperature;
  if (temp < 0) {
    flag |= FLAG_NEG_TEMPR;
    temp = -temp;
  }
  result.data[byte_index++] = (temp >> 8) & 0xFF;
  result.data[byte_index++] = temp & 0xFF;

  result.data[byte_index++] = (new_data.bsec_data.humidity >> 8) & 0xFF;
  result.data[byte_index++] = new_data.bsec_data.humidity & 0xFF;

  result.data[byte_index++] = (new_data.bsec_data.pressure >> 24) & 0xFF;
  result.data[byte_index++] = (new_data.bsec_data.pressure >> 16) & 0xFF;
  result.data[byte_index++] = (new_data.bsec_data.pressure >> 8) & 0xFF;
  result.data[byte_index++] = new_data.bsec_data.pressure & 0xFF;

  result.data[byte_index++] = (new_data.bsec_data.iaq >> 8) & 0xFF;
  result.data[byte_index++] = new_data.bsec_data.iaq & 0xFF;

  result.data[byte_index++] = new_data.bsec_data.iaqAccuracy;

  result.data[byte_index++] = (new_data.bsec_data.staticIaq >> 8) & 0xFF;
  result.data[byte_index++] = new_data.bsec_data.staticIaq & 0xFF;

  result.data[byte_index++] = (new_data.bsec_data.co2Equivalent >> 8) & 0xFF;
  result.data[byte_index++] = new_data.bsec_data.co2Equivalent & 0xFF;

  result.data[byte_index++] = (new_data.bsec_data.breathVoc >> 8) & 0xFF;
  result.data[byte_index++] = new_data.bsec_data.breathVoc & 0xFF;

  result.data[byte_index++] = new_data.bsec_data.gasPercentage;
  result.data[byte_index++] = ((new_data.bsec_data.stabStatus & 0x0F) << 4) |
                              (new_data.bsec_data.runInStatus & 0x0F);

  result.data[byte_index++] = (new_data.mq135_data.analog >> 4) & 0xFF;
  result.data[byte_index++] = (new_data.mq135_data.analog & 0x0F) << 4 |
                              (new_data.anemo_data >> 8) & 0x0F;
  result.data[byte_index++] = new_data.anemo_data & 0xFF;

  result.status = ENCODER_OK;
  result.flag = flag;
  result.streak = 0;
  result.len = byte_index;

  state.data = new_data;
  state.streak = 0;

  return result;
}

EncoderResult Encoder::encode(SensorData new_data, uint8_t flags) {
  flag_t flag = 0;

  if (flags & ENCODE_NO_DELTA) {
    // don't need delta encoding
    return encode_no_delta(new_data);
  }

  /* Update the deltas and flags */
  if ((flags & ENCODE_NO_ANEMO_DATA) == 0) {
    if (new_data.anemo_data >= state.data.anemo_data) {
      state.delta.anemo_data = new_data.anemo_data - state.data.anemo_data;
    } else {
      state.delta.anemo_data = -new_data.anemo_data + state.data.anemo_data;
      flag |= FLAG_NEG_ANEMO;
    }
  }
  if ((flags & ENCODE_NO_MQ135_DATA) == 0) {
    if (new_data.mq135_data.analog >= state.data.mq135_data.analog) {
      state.delta.mq135_data.analog =
          new_data.mq135_data.analog - state.data.mq135_data.analog;
    } else {
      state.delta.mq135_data.analog =
          -new_data.mq135_data.analog + state.data.mq135_data.analog;
      flag |= FLAG_NEG_MQ135;
    }
  }
  // NOTE: the digital field is unused
  if ((flags & ENCODE_NO_BSEC_DATA) == 0) {
    if (new_data.bsec_data.breathVoc >= state.data.bsec_data.breathVoc) {
      state.delta.bsec_data.breathVoc =
          new_data.bsec_data.breathVoc - state.data.bsec_data.breathVoc;
    } else {
      state.delta.bsec_data.breathVoc =
          -new_data.bsec_data.breathVoc + state.data.bsec_data.breathVoc;
      flag |= FLAG_NEG_BTVOC;
    }
    if (new_data.bsec_data.co2Equivalent >=
        state.data.bsec_data.co2Equivalent) {
      state.delta.bsec_data.co2Equivalent =
          new_data.bsec_data.co2Equivalent - state.data.bsec_data.co2Equivalent;
    } else {
      state.delta.bsec_data.co2Equivalent = -new_data.bsec_data.co2Equivalent +
                                            state.data.bsec_data.co2Equivalent;
      flag |= FLAG_NEG_CO2EQ;
    }
    if (new_data.bsec_data.iaq >= state.data.bsec_data.iaq) {
      state.delta.bsec_data.iaq =
          new_data.bsec_data.iaq - state.data.bsec_data.iaq;
    } else {
      state.delta.bsec_data.iaq =
          -new_data.bsec_data.iaq + state.data.bsec_data.iaq;
      flag |= FLAG_NEG_IAQ;
    }

    /* NOTE: the iaq accuracy is already 1 byte, (using as it is)*/
    /* NOTE: using stab and run in status as it is */

    if (new_data.bsec_data.staticIaq >= state.data.bsec_data.staticIaq) {
      state.delta.bsec_data.staticIaq =
          new_data.bsec_data.staticIaq - state.data.bsec_data.staticIaq;
    } else {
      state.delta.bsec_data.staticIaq =
          -new_data.bsec_data.staticIaq + state.data.bsec_data.staticIaq;
      flag |= FLAG_NEG_STIAQ;
    }

    /* NOTE: gas percentage is just 1 byte, using as it is*/

    if (new_data.bsec_data.temperature >= state.data.bsec_data.temperature) {
      state.delta.bsec_data.temperature =
          new_data.bsec_data.temperature - state.data.bsec_data.temperature;
    } else {
      state.delta.bsec_data.temperature =
          -new_data.bsec_data.temperature + state.data.bsec_data.temperature;
      flag |= FLAG_NEG_TEMPR;
    }
    if (new_data.bsec_data.humidity >= state.data.bsec_data.humidity) {
      state.delta.bsec_data.humidity =
          new_data.bsec_data.humidity - state.data.bsec_data.humidity;
    } else {
      state.delta.bsec_data.humidity =
          -new_data.bsec_data.humidity + state.data.bsec_data.humidity;
      flag |= FLAG_NEG_HUMID;
    }
    if (new_data.bsec_data.pressure >= state.data.bsec_data.pressure) {
      state.delta.bsec_data.pressure =
          new_data.bsec_data.pressure - state.data.bsec_data.pressure;
    } else {
      state.delta.bsec_data.pressure =
          -new_data.bsec_data.pressure + state.data.bsec_data.pressure;
      flag |= FLAG_NEG_PRESR;
    }
  }

  /* Update the state */
  state.data = new_data;
  /* Update the streak */
  state.streak++;

  /* Return the result */
  EncoderResult result;

  /* Check the bounds of the delta and fill the result.data buffer */
  /* NOTE: the deltas should be within one byte range*/

  uint8_t byte_index = 0;
  if ((flag & ENCODE_NO_BSEC_DATA) == 0) {
    if ((state.delta.bsec_data.temperature & 0xFF00) == 0) {
      result.data[byte_index++] = state.delta.bsec_data.temperature & 0xFF;
      flag |= FLAG_DELTA_TEMPR;
    } else {
      result.data[byte_index++] =
          (state.delta.bsec_data.temperature >> 8) & 0xFF;
      result.data[byte_index++] = state.delta.bsec_data.temperature & 0xFF;
    }
    if ((state.delta.bsec_data.humidity & 0xFF00) == 0) {
      result.data[byte_index++] = state.delta.bsec_data.humidity & 0xFF;
      flag |= FLAG_DELTA_HUMID;
    } else {
      result.data[byte_index++] = (state.delta.bsec_data.humidity >> 8) & 0xFF;
      result.data[byte_index++] = state.delta.bsec_data.humidity & 0xFF;
    }
    if ((state.delta.bsec_data.pressure & 0xFFFFFF00) == 0) {
      result.data[byte_index++] = state.delta.bsec_data.pressure & 0xFF;
      flag |= FLAG_DELTA_PRESR;
    } else {
      result.data[byte_index++] = (state.delta.bsec_data.pressure >> 24) & 0xFF;
      result.data[byte_index++] = (state.delta.bsec_data.pressure >> 16) & 0xFF;
      result.data[byte_index++] = (state.delta.bsec_data.pressure >> 8) & 0xFF;
      result.data[byte_index++] = state.delta.bsec_data.pressure & 0xFF;
    }
    if ((state.delta.bsec_data.iaq & 0xFF00) == 0) {
      result.data[byte_index++] = state.delta.bsec_data.iaq & 0xFF;
      flag |= FLAG_DELTA_IAQ;
    } else {
      result.data[byte_index++] = (state.delta.bsec_data.iaq >> 8) & 0xFF;
      result.data[byte_index++] = state.delta.bsec_data.iaq & 0xFF;
    }
    /* NOTE: simply place iaq accuracy*/
    result.data[byte_index++] = new_data.bsec_data.iaqAccuracy;
    if ((state.delta.bsec_data.staticIaq & 0xFF00) == 0) {
      result.data[byte_index++] = state.delta.bsec_data.staticIaq & 0xFF;
      flag |= FLAG_DELTA_STIAQ;
    } else {
      result.data[byte_index++] = (state.delta.bsec_data.staticIaq >> 8) & 0xFF;
      result.data[byte_index++] = state.delta.bsec_data.staticIaq & 0xFF;
    }
    if ((state.delta.bsec_data.co2Equivalent & 0xFF00) == 0) {
      result.data[byte_index++] = state.delta.bsec_data.co2Equivalent & 0xFF;
      flag |= FLAG_DELTA_CO2EQ;
    } else {
      result.data[byte_index++] =
          (state.delta.bsec_data.co2Equivalent >> 8) & 0xFF;
      result.data[byte_index++] = state.delta.bsec_data.co2Equivalent & 0xFF;
    }
    if ((state.delta.bsec_data.breathVoc & 0xFF00) == 0) {
      result.data[byte_index++] = state.delta.bsec_data.breathVoc & 0xFF;
      flag |= FLAG_DELTA_BTVOC;
    } else {
      result.data[byte_index++] = (state.delta.bsec_data.breathVoc >> 8) & 0xFF;
      result.data[byte_index++] = state.delta.bsec_data.breathVoc & 0xFF;
    }
    /* NOTE: encode gas percentage directly */
    result.data[byte_index++] = new_data.bsec_data.gasPercentage & 0xFF;
    result.data[byte_index++] = ((new_data.bsec_data.stabStatus & 0x0F) << 4) |
                                (new_data.bsec_data.runInStatus & 0x0F);
  }

  if ((state.delta.mq135_data.analog & 0xFFF0) == 0 &&
      (state.delta.anemo_data & 0xFFF0) == 0) {
    result.data[byte_index++] = ((state.delta.mq135_data.analog & 0x0F) << 4) |
                                (state.delta.anemo_data & 0x0F);
    flag |= FLAG_DELTA_MQ135;
    flag |= FLAG_DELTA_ANEMO;
  } else {
    /* fit the two 12 bit values in 3 byte */
    result.data[byte_index++] = (state.delta.mq135_data.analog >> 4) & 0xFF;
    result.data[byte_index++] = (state.delta.mq135_data.analog & 0x0F) |
                                ((state.delta.anemo_data >> 8) & 0x0F);
    result.data[byte_index++] = state.delta.anemo_data & 0xFF;
  }

  result.status = ENCODER_OK;
  result.flag = flag;
  result.streak = state.streak;
  result.len = byte_index;
  return result;
}
