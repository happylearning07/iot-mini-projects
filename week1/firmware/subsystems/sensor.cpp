#include "sensor.h"
#include <Wire.h>

Sensor *Sensor::instance = nullptr;

bool Sensor::setup() {

    instance = this; // initialize the singleton instance

    pinMode(PIN_MQ135_D0, INPUT);

    // begin the I2C communication
    Wire1.begin(PIN_BME680_SDA, PIN_BME680_SCL);
    Wire1.beginTransmission(I2C_ADDR_BME680);
    if (Wire1.endTransmission() != 0) {
        return false;
    }

    // begin the bsec internal controls
    if (!bsec.begin(I2C_ADDR_BME680, Wire1)) {
        return false;
    }

    // specify the readings that are to obtained per callback
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

    // update the subscription of the bsec handler with low-power sample rate
    if (!bsec.updateSubscription(sensorList, ARRAY_LEN(sensorList),
                                 BSEC_SAMPLE_RATE_LP)) {
        return false;
    }

    bsec.attachCallback(
        bsec_callback); // add the static callback of the class to bsec instance
    return true;
}

void Sensor::bsec_callback(const bme68xData data, const bsecOutputs outputs,
                           Bsec2 bsec) {
    if (instance !=
        nullptr) { // protection against uninitialized instance of Sensor
        instance->process_bsec_outputs(outputs);
    }
}

/* function to process the bsec output and store in the structured placeholder
 * of the Sensor instance */
void Sensor::process_bsec_outputs(const bsecOutputs &outputs) {
    if (!outputs.nOutputs) {
        return;
    }

    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData &output = outputs.output[i];

        switch (output.sensor_id) {
        case BSEC_OUTPUT_IAQ:
            latest_data.bsec_data.iaq = (uint16_t)output.signal;
            latest_data.bsec_data.iaqAccuracy = output.accuracy;
            break;

        case BSEC_OUTPUT_STATIC_IAQ:
            latest_data.bsec_data.staticIaq = (uint16_t)output.signal;
            break;

        case BSEC_OUTPUT_CO2_EQUIVALENT:
            latest_data.bsec_data.co2Equivalent = (uint16_t)output.signal;
            break;

        case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
            latest_data.bsec_data.breathVoc = (uint16_t)(output.signal * 100);
            break;

        case BSEC_OUTPUT_RAW_PRESSURE:
            latest_data.bsec_data.pressure = (uint32_t)(output.signal);
            break;

        case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
            latest_data.bsec_data.temperature = (int16_t)(output.signal * 100);
            break;

        case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
            latest_data.bsec_data.humidity = (uint16_t)(output.signal * 100);
            break;

        case BSEC_OUTPUT_STABILIZATION_STATUS:
            latest_data.bsec_data.stabStatus = (uint8_t)output.signal;
            break;

        case BSEC_OUTPUT_RUN_IN_STATUS:
            latest_data.bsec_data.runInStatus = (uint8_t)output.signal;
            break;

        case BSEC_OUTPUT_GAS_PERCENTAGE:
            latest_data.bsec_data.gasPercentage = (uint8_t)output.signal;
            break;

        default:
            break;
        }
    }

    bsec_data_ready = true;
}

void Sensor::run(uint16_t dt) {
    t += dt;
    if (t >= cadence) {
        t = 0;
        bsec.run();

        /* update the other data fields */
        latest_data.mq135_data.digital = get_mq135_digital();
        latest_data.mq135_data.analog = get_mq135_analog();
        latest_data.anemo_data = get_anemo_analog();
    }
}

bool Sensor::has_new_bsec_data() { return bsec_data_ready; }
bool Sensor::has_bsec_error() {
    return (bsec.status < BSEC_OK) || (bsec.sensor.status < BME68X_OK);
}

uint16_t Sensor::get_mq135_analog() {
    uint16_t res = 0;
    res = analogRead(PIN_MQ135_A0);
    return res;
}

uint8_t Sensor::get_mq135_digital() {
    uint8_t res = 0;
    res = digitalRead(PIN_MQ135_D0);
    return res;
}

uint16_t Sensor::get_anemo_analog() {
    uint16_t res = 0;
    res = analogRead(PIN_ANEMO_A0);
    return res;
}

SensorData Sensor::get_data() {
    if (bsec_data_ready) {
        bsec_data_ready = false;
    } else {
        // NOTE: current default behaviour: do not retain old data. ( set it to
        // zero instead )
        memset(&latest_data.bsec_data, 0, sizeof(BsecData));
    }
    return latest_data;
}
