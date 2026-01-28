#ifndef TRANSMISSION_H_
#define TRANSMISSION_H_

#include "subsystem.h"
#include <LoRaWan_APP.h>

/* Transmission Configuration Macros */
#define RF_FREQUENCY 865000000
#define TX_OUTPUT_POWER 21
#define LORA_BANDWIDTH 0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 1000

class Transmission : public Subsystem {
  private:
    int16_t last_rssi;
    int8_t last_snr;

    static void on_tx_done();
    static void on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi,
                           int8_t snr);
    static void on_tx_timeout();
    static void on_rx_timeout();

    static Transmission *instance;
    RadioEvents_t radio_events;

  public:
    bool setup();
    void run(uint16_t t);
    void transmit(uint8_t *buffer, uint16_t len);
};

#endif // TRANSMISSION_H_
