#ifndef SENSOR_SUITE_H
#define SENSOR_SUITE_H

#include <Arduino.h>
#include "LightSensor.h"
#include "ProximitySensor.h"

struct SensorReadings {
    uint16_t proximityRaw;
    uint16_t lightRaw;
    float lightLux;
    bool valid;
};

class SensorSuite {
public:
    bool begin(bool configureI2C = true, bool verbose = false);
    SensorReadings read(bool verbose = false);
    bool isReady() const;

private:
    ProximitySensor proximitySensor;
    LightSensor lightSensor;
    bool ready = false;

    static constexpr int DEFAULT_I2C_ADDRESS = 0x60;
    static constexpr int DEFAULT_I2C_FREQ = 100000;
    static constexpr int DEFAULT_SDA_PIN = 21;
    static constexpr int DEFAULT_SCL_PIN = 22;
};

#endif
