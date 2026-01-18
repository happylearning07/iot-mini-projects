#ifndef SENSORINTERFACE_H_
#define SENSORINTERFACE_H_

#include <Arduino.h>
#include <Wire.h>
#include <bsec2.h>

/* Default I2C pins for BME680 */
#define DEFAULT_BME680_SDA 41
#define DEFAULT_BME680_SCL 42
#define DEFAULT_BME680_ADDRESS 0x77

/**
 * @brief BSEC2 sensor data structure
 * Contains all processed outputs from BSEC library
 */
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

/**
 * @brief SensorInterface class wrapping BSEC2 library
 *
 * Provides a clean abstraction over the Bosch BSEC2 library for
 * BME680/BME688 environmental sensors with IAQ processing.
 */
class SensorInterface {
private:
  uint8_t sda;
  uint8_t scl;
  uint8_t address;
  float tempOffset;

  Bsec2 bsec;
  BsecData latestData;
  volatile bool dataReady;

  static SensorInterface *instance;
  static void bsecCallback(const bme68xData data, const bsecOutputs outputs,
                           Bsec2 bsec);
  void processOutputs(const bsecOutputs &outputs);

public:
  /**
   * @brief Construct with default I2C pins
   */
  SensorInterface();

  /**
   * @brief Construct with custom I2C pins
   */
  SensorInterface(uint8_t sda, uint8_t scl,
                  uint8_t address = DEFAULT_BME680_ADDRESS);

  ~SensorInterface() {}

  /**
   * @brief Initialize BSEC2 and subscribe to outputs
   * @param sampleRate BSEC_SAMPLE_RATE_LP (3s) or BSEC_SAMPLE_RATE_ULP (5min)
   * @param tempOffset Temperature offset for self-heating compensation
   * @return true if initialization successful
   */
  bool init(float sampleRate = BSEC_SAMPLE_RATE_LP, float tempOffset = 0.0f);

  /**
   * @brief Run BSEC processing (call frequently in loop)
   * @return true if run successful, false on error
   */
  bool run();

  /**
   * @brief Check if new data is available
   * @return true if new BSEC data ready since last read
   */
  bool hasNewData();

  /**
   * @brief Get latest BSEC sensor data
   * Clears the new data flag after reading
   * @return BsecData structure with all sensor values
   */
  BsecData getData();

  /**
   * @brief Get BSEC library version string
   */
  String getVersion();

  /**
   * @brief Get BSEC status code
   */
  bsec_library_return_t getStatus() { return bsec.status; }

  /**
   * @brief Get BME68x sensor status code
   */
  int8_t getSensorStatus() { return bsec.sensor.status; }

  /**
   * @brief Check if BSEC has an error (status < 0)
   */
  bool hasError();

  /**
   * @brief Print status to Serial (for debugging)
   */
  void printStatus();
};

#endif // SENSORINTERFACE_H_
