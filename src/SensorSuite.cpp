#include "SensorSuite.h"
#include "I2C_RW.h"

bool SensorSuite::begin(bool configureI2C, bool verbose) {
    if (configureI2C) {
        I2C_RW::initI2C(DEFAULT_I2C_ADDRESS, DEFAULT_I2C_FREQ, DEFAULT_SDA_PIN, DEFAULT_SCL_PIN);
    }

    bool proxOk = proximitySensor.begin(verbose);
    bool lightOk = lightSensor.begin(verbose);
    ready = proxOk && lightOk;
    return ready;
}

SensorReadings SensorSuite::read(bool verbose) {
    SensorReadings readings{};
    if (!ready) {
        readings.proximityRaw = 0;
        readings.lightRaw = 0;
        readings.lightLux = -1.0f;
        readings.valid = false;
        return readings;
    }

    readings.proximityRaw = proximitySensor.readRaw(verbose);
    readings.lightRaw = lightSensor.readRaw(verbose);
    readings.lightLux = (readings.lightRaw == 0xFFFF) ? -1.0f : (readings.lightRaw * 0.1f);
    readings.valid = (readings.proximityRaw != 0xFFFF && readings.lightRaw != 0xFFFF);
    return readings;
}

bool SensorSuite::isReady() const {
    return ready;
}
