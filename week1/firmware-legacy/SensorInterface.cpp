#include "SensorInterface.h"

// Static instance pointer for callback
SensorInterface *SensorInterface::instance = nullptr;

SensorInterface::SensorInterface()
    : sda(DEFAULT_BME680_SDA), scl(DEFAULT_BME680_SCL),
      address(DEFAULT_BME680_ADDRESS), tempOffset(0.0f), dataReady(false) {
  memset(&latestData, 0, sizeof(latestData));
}

SensorInterface::SensorInterface(uint8_t sda, uint8_t scl, uint8_t address)
    : sda(sda), scl(scl), address(address), tempOffset(0.0f), dataReady(false) {
  memset(&latestData, 0, sizeof(latestData));
}

bool SensorInterface::init(float sampleRate, float tempOffset) {
  this->tempOffset = tempOffset;
  instance = this;

  Serial.println("[SensorInterface] Initializing I2C on Wire1...");
  Wire1.begin(sda, scl);

  // Scan for device
  Wire1.beginTransmission(address);
  if (Wire1.endTransmission() != 0) {
    Serial.printf("[SensorInterface] No device at 0x%02X\n", address);
    return false;
  }
  Serial.printf("[SensorInterface] Found device at 0x%02X\n", address);

  // Initialize BSEC2
  Serial.println("[SensorInterface] Initializing BSEC2...");
  if (!bsec.begin(address, Wire1)) {
    Serial.println("[SensorInterface] BSEC2 init failed!");
    printStatus();
    return false;
  }

  // Set temperature offset
  bsec.setTemperatureOffset(tempOffset);

  // Subscribe to BSEC outputs
  bsecSensor sensorList[] = {BSEC_OUTPUT_IAQ,
                             BSEC_OUTPUT_RAW_TEMPERATURE,
                             BSEC_OUTPUT_RAW_PRESSURE,
                             BSEC_OUTPUT_RAW_HUMIDITY,
                             BSEC_OUTPUT_RAW_GAS,
                             BSEC_OUTPUT_STABILIZATION_STATUS,
                             BSEC_OUTPUT_RUN_IN_STATUS,
                             BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
                             BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
                             BSEC_OUTPUT_STATIC_IAQ,
                             BSEC_OUTPUT_CO2_EQUIVALENT,
                             BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
                             BSEC_OUTPUT_GAS_PERCENTAGE,
                             BSEC_OUTPUT_COMPENSATED_GAS};

  if (!bsec.updateSubscription(sensorList, ARRAY_LEN(sensorList), sampleRate)) {
    Serial.println("[SensorInterface] Subscription update failed!");
    printStatus();
    return false;
  }

  // Attach callback
  bsec.attachCallback(bsecCallback);

  Serial.println("[SensorInterface] BSEC2 initialized successfully");
  Serial.println("[SensorInterface] Version: " + getVersion());

  return true;
}

bool SensorInterface::run() { return bsec.run(); }

bool SensorInterface::hasNewData() { return dataReady; }

BsecData SensorInterface::getData() {
  dataReady = false;
  return latestData;
}

String SensorInterface::getVersion() {
  return String(bsec.version.major) + "." + String(bsec.version.minor) + "." +
         String(bsec.version.major_bugfix) + "." +
         String(bsec.version.minor_bugfix);
}

bool SensorInterface::hasError() {
  return (bsec.status < BSEC_OK) || (bsec.sensor.status < BME68X_OK);
}

void SensorInterface::printStatus() {
  if (bsec.status < BSEC_OK) {
    Serial.printf("[SensorInterface] BSEC error: %d\n", bsec.status);
  } else if (bsec.status > BSEC_OK) {
    Serial.printf("[SensorInterface] BSEC warning: %d\n", bsec.status);
  }

  if (bsec.sensor.status < BME68X_OK) {
    Serial.printf("[SensorInterface] BME68X error: %d\n", bsec.sensor.status);
  } else if (bsec.sensor.status > BME68X_OK) {
    Serial.printf("[SensorInterface] BME68X warning: %d\n", bsec.sensor.status);
  }
}

// Static callback - routes to instance method
void SensorInterface::bsecCallback(const bme68xData data,
                                   const bsecOutputs outputs, Bsec2 bsec) {
  if (instance != nullptr) {
    instance->processOutputs(outputs);
  }
}

// Process BSEC outputs and store in latestData
void SensorInterface::processOutputs(const bsecOutputs &outputs) {
  if (!outputs.nOutputs) {
    return;
  }

  for (uint8_t i = 0; i < outputs.nOutputs; i++) {
    const bsecData &output = outputs.output[i];

    switch (output.sensor_id) {
    case BSEC_OUTPUT_IAQ:
      latestData.iaq = (uint16_t)output.signal;
      latestData.iaqAccuracy = output.accuracy;
      break;

    case BSEC_OUTPUT_STATIC_IAQ:
      latestData.staticIaq = (uint16_t)output.signal;
      break;

    case BSEC_OUTPUT_CO2_EQUIVALENT:
      latestData.co2Equivalent = (uint16_t)output.signal;
      break;

    case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
      latestData.breathVoc = (uint16_t)(output.signal * 100);
      break;

    case BSEC_OUTPUT_RAW_PRESSURE:
      latestData.pressure = (uint32_t)(output.signal);
      break;

    case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
      latestData.temperature = (int16_t)(output.signal * 100);
      break;

    case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
      latestData.humidity = (uint16_t)(output.signal * 100);
      break;

    case BSEC_OUTPUT_STABILIZATION_STATUS:
      latestData.stabStatus = (uint8_t)output.signal;
      break;

    case BSEC_OUTPUT_RUN_IN_STATUS:
      latestData.runInStatus = (uint8_t)output.signal;
      break;

    case BSEC_OUTPUT_GAS_PERCENTAGE:
      latestData.gasPercentage = (uint8_t)output.signal;
      break;

    default:
      break;
    }
  }

  dataReady = true;
}

// Read analog sensors
void SensorInterface::readAnalogSensors() {
  latestData.mq135Raw = analogRead(ANALOG_MQ135_PIN);
  latestData.anemometerRaw = analogRead(ANALOG_ANEMOMETER_PIN);
}
