#ifndef SENSOR_H_
#define SENSOR_H_

#include "../meta.h"
#include "subsystem.h"
#include <Arduino.h>
#include <bsec2.h>

/* macro definition */
// pins
#define PIN_MQ135_A0 6
#define PIN_MQ135_D0 2
#define PIN_ANEMO_A0 4
#define PIN_BME680_SDA 41
#define PIN_BME680_SCL 42

// defaults
#define I2C_ADDR_BME680 0x77

/* end macro definition */

class Sensor : public Subsystem {
  private:
    uint16_t cadence; // frequency to run the update
    uint16_t t;       // time since last update
    Bsec2 bsec;       // bsec wrapper object

    SensorData latest_data; // place for callback to act on

    volatile bool bsec_data_ready;

    static Sensor *instance; // for singleton class
    // the callback function for the bme680 sensor readings
    static void bsec_callback(const bme68xData data, const bsecOutputs outputs,
                              Bsec2 bsec);
    void process_bsec_outputs(const bsecOutputs &outputs);

    uint16_t get_mq135_analog();
    uint8_t get_mq135_digital();
    uint16_t get_anemo_analog();

  public:
    bool setup();
    void run(uint16_t dt);

    // bsec runtime info routines
    bool has_new_bsec_data();
    bool has_bsec_error();
    SensorData get_data();
};

#endif // SENSOR_H_
