#include "I2C_RW.h"

MockSerialClass Serial;

int I2C_RW::i2cAddress = 0;
int I2C_RW::i2cFrequency = 0;
int I2C_RW::i2cSdaPin = 0;
int I2C_RW::i2cSclPin = 0;

std::unordered_map<uint8_t, uint16_t> I2C_RW::mockReadValues;
std::unordered_map<uint8_t, uint16_t> I2C_RW::mockWriteValues;

void I2C_RW::initI2C(int i2cAddr, int i2cFreq, int pinSda, int pinScl) {
    i2cAddress = i2cAddr;
    i2cFrequency = i2cFreq;
    i2cSdaPin = pinSda;
    i2cSclPin = pinScl;
}

void I2C_RW::scanI2cLinesForAddresses(bool) {}
void I2C_RW::printI2cReturnStatus(byte, int, const char[]) {}

uint16_t I2C_RW::readReg8Addr16Data(byte regAddr, int, String, bool) {
    auto it = mockReadValues.find(regAddr);
    if (it == mockReadValues.end()) {
        return 0xFFFF;
    }
    return it->second;
}

bool I2C_RW::writeReg8Addr16DataWithProof(byte regAddr, int, uint16_t data, String, bool) {
    mockWriteValues[regAddr] = data;
    mockReadValues[regAddr] = data;
    return true;
}

void I2C_RW::writeReg8Addr16Data(byte regAddr, uint16_t data, String, bool) {
    mockWriteValues[regAddr] = data;
}

void I2C_RW::resetMock() {
    mockReadValues.clear();
    mockWriteValues.clear();
    i2cAddress = 0;
    i2cFrequency = 0;
    i2cSdaPin = 0;
    i2cSclPin = 0;
}

void I2C_RW::setReadValue(uint8_t regAddr, uint16_t value) {
    mockReadValues[regAddr] = value;
}

uint16_t I2C_RW::getLastWriteValue(uint8_t regAddr) {
    auto it = mockWriteValues.find(regAddr);
    if (it == mockWriteValues.end()) {
        return 0xFFFF;
    }
    return it->second;
}
