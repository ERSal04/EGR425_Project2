#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

compiler=""
if command -v g++ >/dev/null 2>&1; then
  compiler="g++"
elif command -v g++.exe >/dev/null 2>&1; then
  compiler="g++.exe"
elif command -v clang++ >/dev/null 2>&1; then
  compiler="clang++"
elif command -v clang++.exe >/dev/null 2>&1; then
  compiler="clang++.exe"
elif [[ -x "/c/Program Files/LLVM/bin/clang++.exe" ]]; then
  compiler="/c/Program Files/LLVM/bin/clang++.exe"
elif [[ -x "/c/msys64/ucrt64/bin/g++.exe" ]]; then
  compiler="/c/msys64/ucrt64/bin/g++.exe"
else
  echo "No C++ compiler found (g++/clang++)."
  echo "Tip: install LLVM from winget, then re-run this script."
  exit 1
fi

"$compiler" -std=c++17 -Wall -Wextra -pedantic \
  -Itest/host_sensor/mocks -Iinclude \
  src/ProximitySensor.cpp src/LightSensor.cpp src/SensorSuite.cpp \
  test/host_sensor/mocks/I2C_RW.cpp test/host_sensor/test_main.cpp \
  -o test/host_sensor/host_sensor_tests

./test/host_sensor/host_sensor_tests
