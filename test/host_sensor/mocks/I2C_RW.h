#ifndef I2C_RW_H
#define I2C_RW_H

#include "Arduino.h"
#include <cstdint>
#include <unordered_map>

class I2C_RW {
public:
    static int i2cAddress;
    static int i2cFrequency;
    static int i2cSdaPin;
    static int i2cSclPin;

    static void initI2C(int i2cAddr, int i2cFreq, int pinSda, int pinScl);
    static void scanI2cLinesForAddresses(bool verboseConnectionFailures);
    static void printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]);

    static uint16_t readReg8Addr16Data(byte regAddr, int numBytesToRead, String action, bool verbose);
    static bool writeReg8Addr16DataWithProof(byte regAddr, int numBytesToWrite, uint16_t data, String action, bool verbose);
    static void writeReg8Addr16Data(byte regAddr, uint16_t data, String action, bool verbose);

    static void resetMock();
    static void setReadValue(uint8_t regAddr, uint16_t value);
    static uint16_t getLastWriteValue(uint8_t regAddr);

private:
    static std::unordered_map<uint8_t, uint16_t> mockReadValues;
    static std::unordered_map<uint8_t, uint16_t> mockWriteValues;
};

#endif
