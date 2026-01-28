#include "transmission.h"

Transmission *Transmission::instance = nullptr;

bool Transmission::setup() {
  instance = this;
  radio_events.TxDone = on_tx_done;
  radio_events.TxTimeout = on_tx_timeout;
  radio_events.RxDone = on_rx_done;
  radio_events.RxTimeout = on_rx_timeout;

  Radio.Init(&radio_events);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON, true, 0,
                    0, LORA_IQ_INVERSION_ON, 3000);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_FIX_LENGTH_PAYLOAD_ON, 0, true, 0, 0,
                    LORA_IQ_INVERSION_ON, true, RX_TIMEOUT_VALUE);
  Radio.Rx(0);
  return true;
}

void Transmission::transmit(uint8_t *buffer, uint16_t len) {
  Radio.Send(buffer, len);
}

void Transmission::run(uint16_t dt) {
  (void)dt;
  Radio.IrqProcess();
}

void Transmission::on_tx_done() {
  if (instance) {
    Radio.Rx(0);
  }
}

void Transmission::on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi,
                              int8_t snr) {
  if (instance) {
    instance->last_rssi = rssi;
    instance->last_snr = snr;
    Radio.Rx(0);
  }
}

void Transmission::on_tx_timeout() {
  if (instance) {
    Radio.Rx(0);
  }
}

void Transmission::on_rx_timeout() {
  if (instance) {
    Radio.Rx(0);
  }
}
