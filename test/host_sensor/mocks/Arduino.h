#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>
#include <string>

using byte = uint8_t;
using String = std::string;

struct MockSerialClass {
    void println(const char*) {}
    void println(const String&) {}
};

extern MockSerialClass Serial;

#endif
