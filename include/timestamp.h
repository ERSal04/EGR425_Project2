#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <M5Core2.h>
#include <WiFi.h>


// NTP Client setup
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

// Function declarations
void initTimeClient();
void updateTime();
String getFormattedTime();
String getLastUpdateTime();
void drawTimeStamp(int x, int y, uint16_t color);

#endif