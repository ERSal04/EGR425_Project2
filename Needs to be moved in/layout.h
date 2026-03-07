#ifndef LAYOUT_H
#define LAYOUT_H

#include <Arduino.h>
#include <M5Core2.h>
#include "EGR425_Phase1_weather_bitmap_images.h"
#include "timestamp.h"

extern int sHeight;
extern int sWidth;

void drawGradientStyle(double tempNow, double tempMin, double tempMax, String cityName, String weatherDesc, String iconId, uint16_t primaryTextColor, String tempChar);
void getScreenMetrics();
void drawWeatherImage(String iconId, int resizeMult);

#endif