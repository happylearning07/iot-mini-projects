#include "SensorInterface.h"
#include "display.h"
#include "lora_config.h"
#include "packet.h"
#include <Arduino.h>
#include <LoRaWan_APP.h>

#define BAUD 115200
#define DEVICE_ID 0xA109
#define LOOP_FREQ 10
#define PANIC_LED LED_BUILTIN
#define ERROR_DUR 1000

static OledDisplay oledDisplay;
static RadioEvents_t radioEvents;
static SensorInterface sensor;
static LoRaPacket packet;
static uint8_t txBuffer[PACKET_SIZE];

// Analog packet globals
static AnalogPacket analogPacket;
static uint8_t analogTxBuffer[ANALOG_PACKET_SIZE];
static uint16_t analogSequence = 0;

static int16_t lastRssi = 0;
static int8_t lastSNR = 0;
static uint16_t packetSequence = 0;

/* Radio callbacks */
static bool loraIdle = true; // Use simple idle flag logic if possible, or
                             // volatile flag managed by callbacks
static volatile bool isTransmitting = false;

static void onTxDone() {
  Serial.println("TX Done");
  Radio.Sleep();
  isTransmitting = false;
}

static void onTxTimeout() {
  Serial.println("TX Timeout");
  Radio.Sleep();
  isTransmitting = false;
}

static void onRxDone(uint8_t *payload, uint16_t size, int16_t rssi,
                     int8_t snr) {
  // Transmitter doesn't expect RX usually but good to stabilize
  Radio.Sleep();
  lastRssi = rssi;
  lastSNR = snr;
  Serial.printf("Received frame: %d bytes\n", size);
}

static char buffer[64];
static void renderDashboard(const BsecData &data);
static void errLeds(void);

/* setup routine */
void setup() {
  Serial.begin(BAUD);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  pinMode(PANIC_LED, OUTPUT);

  Serial.println("Initializing sensor...");
  if (!sensor.init(BSEC_SAMPLE_RATE_ULP, 0.0f)) {
    Serial.println("Sensor initialization failed!");
    errLeds();
  }
  Serial.println("Sensor initialized: BSEC v" + sensor.getVersion());

  Serial.print("Initializing OLED...");
  oledDisplay.init();
  Serial.println("(done)");

  Serial.print("Configuring LoRa radio...");
  radioEvents.TxDone = onTxDone;
  radioEvents.TxTimeout = onTxTimeout;
  radioEvents.RxDone = onRxDone;

  Radio.Init(&radioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON, true, 0,
                    0, LORA_IQ_INVERSION_ON, 3000);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_FIX_LENGTH_PAYLOAD_ON, 0, true, 0, 0,
                    LORA_IQ_INVERSION_ON, true, RX_TIMEOUT_VALUE);
  Serial.println("(done)");

  Radio.Rx(0);

  delay(2000);
  oledDisplay.clear();
  oledDisplay.commit();

  initPacket(&packet, DEVICE_ID);
  initAnalogPacket(&analogPacket, DEVICE_ID);
  Serial.println("Setup complete. Waiting for sensor data...");
}

/* loop routine */
void loop() {
  Radio.IrqProcess(); // Ensure radio callbacks run

  // Run sensor processing (BSEC)
  if (!sensor.run()) {
    if (sensor.hasError()) {
      sensor.printStatus();
      errLeds();
    }
  }

  sensor.readAnalogSensors();
  BsecData data = sensor.peekData(); // Use peek to NOT clear the flag

  // --- Analog Packet Transmission (Every 10ms) ---
  static uint32_t lastAnalogTx = 0;
  if (millis() - lastAnalogTx >= 10) {
    if (!isTransmitting) {
      populateAnalogPacket(&analogPacket, analogSequence++, millis() / 1000,
                           data.mq135Raw, data.anemometerRaw);

      uint8_t len = encodeAnalogPacket(analogPacket, analogTxBuffer);
      isTransmitting = true;
      Radio.Send(analogTxBuffer, len);
      lastAnalogTx = millis();
      // printAnalogPacket(analogPacket); // Reduce serial spam
    }
  }

  // --- Display Update (Throttled to ~250ms) ---
  static uint32_t lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 250) {
    oledDisplay.clearLine(4);
    sprintf(buffer, "MQ: %d  AN: %d", data.mq135Raw, data.anemometerRaw);
    oledDisplay.write(0, 4, buffer);
    oledDisplay.commit();
    lastDisplayUpdate = millis();
  }

  // --- Environmental Packet Transmission (When BSEC Updates) ---
  if (sensor.hasNewData()) {
    // Wait for radio to be free (Env is priority)
    uint32_t waitStart = millis();
    while (isTransmitting && (millis() - waitStart < 1000)) {
      Radio.IrqProcess();
      delay(1);
    }

    if (!isTransmitting) {
      data = sensor.getData(); // NOW we clear the flag and get full data
      renderDashboard(data);   // Full update (clears screen)

      populatePacket(&packet, packetSequence++, millis() / 1000,
                     data.temperature, data.humidity, data.pressure, data.iaq,
                     data.iaqAccuracy, data.staticIaq, data.co2Equivalent,
                     data.breathVoc, data.gasPercentage, data.stabStatus,
                     data.runInStatus);

      uint8_t len = encodePacket(packet, txBuffer);
      isTransmitting = true;
      Radio.Send(txBuffer, len);
      printPacket(packet);
    } else {
      Serial.println("[WARN] Skipped ENV Tx - Radio Busy");
    }
  }

  Radio.IrqProcess();
  delay(1000 / LOOP_FREQ);
}

/* Error LED blink */
void errLeds(void) {
  while (1) {
    digitalWrite(PANIC_LED, HIGH);
    delay(ERROR_DUR);
    digitalWrite(PANIC_LED, LOW);
    delay(ERROR_DUR);
  }
}

/* Show uptime */
static void showTime() {
  uint32_t uptime = millis() / 1000;
  uint32_t hours = uptime / 3600;
  uint32_t minutes = (uptime % 3600) / 60;
  uint32_t seconds = uptime % 60;
  sprintf(buffer, "U:%02d:%02d:%02d", hours, minutes, seconds);
  oledDisplay.write(6, 2, buffer);
}

/* Draw banner */
static void drawBanner() { oledDisplay.write(0, 0, " ENV MONITOR "); }

/* Draw analog sensor data (replaces network status) */
static void drawAnalog(const BsecData &data) {
  sprintf(buffer, "MQ: %d  AN: %d", data.mq135Raw, data.anemometerRaw);
  oledDisplay.write(0, 4, buffer);
}

/* Draw sensor data */
static void drawSensors(const BsecData &data) {
  // Line 1: Temperature and Humidity
  sprintf(buffer, "T:%d.%02dC H:%d.%02d%%", data.temperature / 100,
          abs(data.temperature % 100), data.humidity / 100,
          data.humidity % 100);
  oledDisplay.write(0, 1, buffer);

  // Line 2: Pressure
  sprintf(buffer, "MPa:%d.%03d", data.pressure / 1000, (data.pressure) % 1000);
  oledDisplay.write(0, 2, buffer);

  // Line 3: IAQ with accuracy and CO2
  const char *iaqLabel;
  if (data.iaq <= 50)
    iaqLabel = "Good";
  else if (data.iaq <= 100)
    iaqLabel = "OK";
  else if (data.iaq <= 150)
    iaqLabel = "Fair";
  else if (data.iaq <= 200)
    iaqLabel = "Poor";
  else if (data.iaq <= 300)
    iaqLabel = "Bad";
  else
    iaqLabel = "Hazard";

  sprintf(buffer, "IAQ:%d(%d)%s CO2:%d", data.iaq, data.iaqAccuracy, iaqLabel,
          data.co2Equivalent);
  oledDisplay.write(0, 3, buffer);
}

/* Render full dashboard */
static void renderDashboard(const BsecData &data) {
  oledDisplay.clear();
  drawBanner();
  drawSensors(data);
  drawAnalog(data);
  showTime();
  oledDisplay.commit();
}
