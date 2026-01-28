/**
 *  @file meta.h
 *  @brief Definitions of common Metadata Data Structures used in the program
 *  */

#ifndef META_H_
#define META_H_

#include <stddef.h>
#include <stdint.h>

struct BsecData {
    int16_t temperature;
    uint16_t humidity;
    uint32_t pressure;
    uint16_t iaq;
    uint8_t iaqAccuracy;
    uint16_t staticIaq;
    uint16_t co2Equivalent;
    uint16_t breathVoc;
    uint8_t gasPercentage;
    uint8_t stabStatus;
    uint8_t runInStatus;
};

struct Mq135Data {
    uint16_t analog;
    uint8_t digital;
};

struct SensorData {
    BsecData bsec_data;
    Mq135Data mq135_data;
    uint16_t anemo_data;
};

#endif // META_H_
