#ifndef PROXIMITY_SENSOR_H
#define PROXIMITY_SENSOR_H

#include <Arduino.h>

class ProximitySensor {
public:
    bool begin(bool verbose = false);
    uint16_t readRaw(bool verbose = false);

private:
    static constexpr uint8_t PS_CONF1_REG = 0x03;
    static constexpr uint8_t PS_DATA_REG = 0x08;
    static constexpr uint8_t DEVICE_ID_REG = 0x0C;
};

#endif
