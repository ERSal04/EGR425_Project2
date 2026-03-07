#include "ProximitySensor.h"
#include "I2C_RW.h"

bool ProximitySensor::begin(bool verbose) {
    uint16_t deviceId = I2C_RW::readReg8Addr16Data(DEVICE_ID_REG, 2, "Read VCNL4040 device ID", verbose);
    if (deviceId == 0xFFFF) {
        Serial.println("WARN: Proximity sensor not detected.");
        return false;
    }

    uint16_t psConf1 = I2C_RW::readReg8Addr16Data(PS_CONF1_REG, 2, "Read PS_CONF1", verbose);
    if (psConf1 == 0xFFFF) {
        Serial.println("WARN: Could not read proximity configuration.");
        return false;
    }

    psConf1 &= ~0x0001;
    bool configured = I2C_RW::writeReg8Addr16DataWithProof(PS_CONF1_REG, 2, psConf1, "Enable proximity sensing", verbose);
    if (!configured) {
        Serial.println("WARN: Could not enable proximity sensing.");
    }

    return configured;
}

uint16_t ProximitySensor::readRaw(bool verbose) {
    return I2C_RW::readReg8Addr16Data(PS_DATA_REG, 2, "Read proximity data", verbose);
}
