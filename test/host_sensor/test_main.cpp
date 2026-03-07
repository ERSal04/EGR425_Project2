#include <cmath>
#include <iostream>

#include "I2C_RW.h"
#include "LightSensor.h"
#include "ProximitySensor.h"
#include "SensorSuite.h"

namespace {

int failures = 0;

void expectTrue(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        failures++;
    }
}

void expectEqU16(uint16_t actual, uint16_t expected, const char* message) {
    if (actual != expected) {
        std::cerr << "FAIL: " << message << " (actual=" << actual << ", expected=" << expected << ")\n";
        failures++;
    }
}

void expectNear(float actual, float expected, float tolerance, const char* message) {
    if (std::fabs(actual - expected) > tolerance) {
        std::cerr << "FAIL: " << message << " (actual=" << actual << ", expected=" << expected << ")\n";
        failures++;
    }
}

void testProximityBeginAndRead() {
    I2C_RW::resetMock();
    I2C_RW::setReadValue(0x0C, 0x0186);
    I2C_RW::setReadValue(0x03, 0x0007);
    I2C_RW::setReadValue(0x08, 4321);

    ProximitySensor proximity;
    bool ok = proximity.begin(false);

    expectTrue(ok, "Proximity begin should succeed with valid device/config regs");
    expectEqU16(I2C_RW::getLastWriteValue(0x03), 0x0006, "Proximity begin should clear shutdown bit in PS_CONF1");
    expectEqU16(proximity.readRaw(false), 4321, "Proximity readRaw should return mocked PS_DATA value");
}

void testLightBeginAndLuxConversion() {
    I2C_RW::resetMock();
    I2C_RW::setReadValue(0x00, 0x0009);
    I2C_RW::setReadValue(0x09, 1234);

    LightSensor light;
    bool ok = light.begin(false);

    expectTrue(ok, "Light begin should succeed with valid config reg");
    expectEqU16(I2C_RW::getLastWriteValue(0x00), 0x0008, "Light begin should clear shutdown bit in ALS_CONF");
    expectEqU16(light.readRaw(false), 1234, "Light readRaw should return mocked ALS_DATA value");
    expectNear(light.readLux(false), 123.4f, 0.001f, "Light readLux should convert raw to lux by 0.1 factor");
}

void testSensorSuiteReadyAndRead() {
    I2C_RW::resetMock();
    I2C_RW::setReadValue(0x0C, 0x0186);
    I2C_RW::setReadValue(0x03, 0x0003);
    I2C_RW::setReadValue(0x08, 2000);
    I2C_RW::setReadValue(0x00, 0x0005);
    I2C_RW::setReadValue(0x09, 350);

    SensorSuite suite;
    bool ready = suite.begin(true, false);
    SensorReadings readings = suite.read(false);

    expectTrue(ready, "SensorSuite begin should report ready when both sensors initialize");
    expectTrue(suite.isReady(), "SensorSuite isReady should be true after successful begin");
    expectTrue(readings.valid, "SensorSuite read should be marked valid with good mocked reads");
    expectEqU16(readings.proximityRaw, 2000, "SensorSuite should return proximity value");
    expectEqU16(readings.lightRaw, 350, "SensorSuite should return light raw value");
    expectNear(readings.lightLux, 35.0f, 0.001f, "SensorSuite should convert light raw to lux");
}

void testSensorSuiteFailurePath() {
    I2C_RW::resetMock();

    SensorSuite suite;
    bool ready = suite.begin(true, false);
    SensorReadings readings = suite.read(false);

    expectTrue(!ready, "SensorSuite begin should fail if sensors are unavailable");
    expectTrue(!suite.isReady(), "SensorSuite isReady should be false after failed begin");
    expectTrue(!readings.valid, "SensorSuite read should be invalid when not ready");
    expectEqU16(readings.proximityRaw, 0, "Failure read should return 0 proximity");
    expectEqU16(readings.lightRaw, 0, "Failure read should return 0 light raw");
    expectNear(readings.lightLux, -1.0f, 0.001f, "Failure read should return -1 lux");
}

} // namespace

int main() {
    testProximityBeginAndRead();
    testLightBeginAndLuxConversion();
    testSensorSuiteReadyAndRead();
    testSensorSuiteFailurePath();

    if (failures == 0) {
        std::cout << "All host sensor tests passed.\n";
        return 0;
    }

    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
