#include "SHT40.h"

bool SHT40::begin(bool verbose) {
    Wire.beginTransmission(I2C_ADDRESS);
    byte err = Wire.endTransmission();
    if (err != 0) {
        Serial.printf("SHT40: not found at 0x44 (err=%d)\n", err);
        return false;
    }
    if (verbose) Serial.println("SHT40: found at 0x44");
    return true;
}

bool SHT40::read(float &tempC, float &humidity, bool verbose) {
    // Send measure command
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(CMD_MEASURE_HI);
    byte err = Wire.endTransmission();
    if (err != 0) {
        Serial.printf("SHT40: write error %d\n", err);
        return false;
    }

    delay(10); // SHT40 needs ~8.3ms for high precision

    // Read 6 bytes: [T_MSB, T_LSB, T_CRC, H_MSB, H_LSB, H_CRC]
    Wire.requestFrom((uint8_t)I2C_ADDRESS, (uint8_t)6);
    if (Wire.available() < 6) {
        Serial.println("SHT40: not enough bytes returned");
        return false;
    }

    uint8_t buf[6];
    for (int i = 0; i < 6; i++) buf[i] = Wire.read();

    // Convert raw values using SHT40 datasheet formulas
    uint16_t rawTemp = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t rawHum  = ((uint16_t)buf[3] << 8) | buf[4];

    tempC    = -45.0f + 175.0f * (rawTemp / 65535.0f);
    humidity = -6.0f  + 125.0f * (rawHum  / 65535.0f);
    humidity = constrain(humidity, 0.0f, 100.0f);

    if (verbose) {
        Serial.printf("SHT40: %.2f°C  %.1f%%RH\n", tempC, humidity);
    }
    return true;
}