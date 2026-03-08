#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>

class LightSensor {
public:
    bool begin(bool verbose = false);
    uint16_t readRaw(bool verbose = false);
    float readLux(bool verbose = false);

private:
    static constexpr uint8_t ALS_CONF_REG = 0x00;
    static constexpr uint8_t ALS_DATA_REG = 0x09;
};

#endif
