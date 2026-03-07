#include "I2C_RW.h"

int I2C_RW::i2cAddress = 0;
int I2C_RW::i2cFrequency = 0;
int I2C_RW::i2cSdaPin = 0;
int I2C_RW::i2cSclPin = 0;

void I2C_RW::initI2C(int i2cAddr, int i2cFreq, int pinSda, int pinScl) {
    i2cAddress = i2cAddr;
    i2cFrequency = i2cFreq;
    i2cSdaPin = pinSda;
    i2cSclPin = pinScl;
    Wire.begin(i2cSdaPin, i2cSclPin, i2cFrequency);
}

void I2C_RW::scanI2cLinesForAddresses(bool verboseConnectionFailures) {
    byte error, address;
    int nDevices;
    Serial.println("STATUS: Scanning for I2C devices...");
    nDevices = 0;

    bool addressesFound[128] = {false};
    for (address = 0; address < 128; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            addressesFound[address] = true;
            nDevices++;
        } else if (verboseConnectionFailures) {
            char message[20];
            sprintf(message, "on address 0x%02X", address);
            (void)message;
        }
    }

    if (nDevices == 0) {
        Serial.println("\tSTATUS: No I2C devices found\n");
    } else {
        Serial.print("\tSTATUS: Done scanning for I2C devices. Devices found at following addresses: \n\t\t");
        for (address = 0; address < 128; address++) {
            if (addressesFound[address]) {
                Serial.printf("0x%02X   ", address);
            }
        }
        Serial.println("");
    }
    delay(100);
}

void I2C_RW::printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]) {
    switch (returnStatus) {
        case 0:
            Serial.printf("\tSTATUS (I2C): %d bytes successfully read/written %s\n", bytesWritten, action);
            break;
        case 1:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - data too long to fit in transmit buffer***\n", bytesWritten, action);
            break;
        case 2:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - received NACK on transmit of address***\n", bytesWritten, action);
            break;
        case 3:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - received NACK on transmit of data***\n", bytesWritten, action);
            break;
        default:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - unknown error***\n", bytesWritten, action);
            break;
    }
}

uint16_t I2C_RW::readReg8Addr16Data(byte regAddr, int numBytesToRead, String action, bool verbose) {
    int maxRetries = 50;
    for (int i = 0; i < maxRetries; i++) {
        Wire.beginTransmission(i2cAddress);

        int bytesWritten = 0;
        bytesWritten += Wire.write(regAddr);

        byte returnStatus = Wire.endTransmission(false);
        if (verbose) {
            printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
        }

        Wire.requestFrom(i2cAddress, numBytesToRead);

        if (Wire.available() == numBytesToRead) {
            uint16_t data = 0;
            uint8_t lsb = Wire.read();
            uint8_t msb = Wire.read();
            data = ((uint16_t)msb << 8 | lsb);

            if (verbose) {
                Serial.printf("\tSTATUS: I2C read at address 0x%02X = 0x%04X (%s)\n", regAddr, data, action.c_str());
            }
            return data;
        }
        delay(10);
    }

    Serial.printf("\tERROR: I2C FAILED to read at address 0x%02X (%s)\n", regAddr, action.c_str());
    return 0xFFFF;
}

void I2C_RW::writeReg8Addr16Data(byte regAddr, uint16_t data, String action, bool verbose) {
    Wire.beginTransmission(i2cAddress);

    int bytesWritten = Wire.write(regAddr);
    bytesWritten += Wire.write(data & 0xFF);
    bytesWritten += Wire.write(data >> 8);
    byte returnStatus = Wire.endTransmission();
    if (verbose) {
        printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
    }
}

bool I2C_RW::writeReg8Addr16DataWithProof(byte regAddr, int numBytesToWrite, uint16_t data, String action, bool verbose) {
    uint16_t existingData = readReg8Addr16Data(regAddr, numBytesToWrite, action, false);
    delay(10);

    int maxRetries = 200;

    for (int i = 0; i < maxRetries; i++) {
        writeReg8Addr16Data(regAddr, data, action, false);
        uint16_t dataRead = readReg8Addr16Data(regAddr, numBytesToWrite, action, false);
        if (dataRead == data) {
            if (verbose) {
                Serial.printf("\tSTATUS: I2C updated REG[0x%02X]: 0x%04X ==> 0x%04X (%s)\n", regAddr, existingData, dataRead, action.c_str());
            }
            return true;
        }
        delay(10);
    }

    Serial.printf("\tERROR: I2C FAILED to write REG[0x%02X]: 0x%04X =X=> 0x%04X (%s)\n", regAddr, existingData, data, action.c_str());
    return false;
}
