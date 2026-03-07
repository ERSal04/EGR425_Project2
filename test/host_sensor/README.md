# Host Sensor Tests (No Device Needed)

These tests validate the new sensor logic without an M5Core2 connected.

## What is tested
- Proximity sensor init (`begin`) and raw read (`readRaw`)
- Light sensor init (`begin`), raw read, and lux conversion (`readLux`)
- SensorSuite success path and failure path handling

## How it works
- Uses mocked `Arduino.h` and mocked `I2C_RW` in `test/host_sensor/mocks/`
- Compiles only sensor modules + test harness as a normal desktop C++ binary

## Run
From workspace root in bash:

```bash
bash test/host_sensor/run_host_sensor_tests.sh
```

If all tests pass, output ends with:

`All host sensor tests passed.`
