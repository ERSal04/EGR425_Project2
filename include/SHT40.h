#ifndef SHT40_H
#define SHT40_H

#include <Arduino.h>
#include <Wire.h>

class SHT40 {
public:
    bool begin(bool verbose = false);
    bool read(float &tempC, float &humidity, bool verbose = false);

private:
    static constexpr uint8_t I2C_ADDRESS   = 0x44;
    static constexpr uint8_t CMD_MEASURE_HI = 0xFD; // High precision
};

#endif