#include "LightSensor.h"
#include "I2C_RW.h"

bool LightSensor::begin(bool verbose) {
    uint16_t alsConf = I2C_RW::readReg8Addr16Data(ALS_CONF_REG, 2, "Read ALS_CONF", verbose);
    if (alsConf == 0xFFFF) {
        Serial.println("WARN: Could not read light sensor configuration.");
        return false;
    }

    alsConf &= ~0x0001;
    bool configured = I2C_RW::writeReg8Addr16DataWithProof(ALS_CONF_REG, 2, alsConf, "Enable ambient light sensing", verbose);
    if (!configured) {
        Serial.println("WARN: Could not enable ambient light sensing.");
    }

    return configured;
}

uint16_t LightSensor::readRaw(bool verbose) {
    return I2C_RW::readReg8Addr16Data(ALS_DATA_REG, 2, "Read ambient light data", verbose);
}

float LightSensor::readLux(bool verbose) {
    uint16_t raw = readRaw(verbose);
    if (raw == 0xFFFF) {
        return -1.0f;
    }
    return raw * 0.1f;
}
