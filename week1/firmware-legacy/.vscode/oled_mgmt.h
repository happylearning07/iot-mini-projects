#ifndef OLED_MGMT_H_
#define OLED_MGMT_H_

#include "config.h"
#include <Arduino.h>
#include <stdint.h>

void VextON() {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
}

#endif // OLED_MGMT_H_
