#include "timestamp.h"

// NTP Client objects
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -28800, 60000);

// Store last update time
String lastUpdateTime = "Never";

void initTimeClient() {
    timeClient.begin();
    Serial.println("NTP Time Client initialized");
}

void updateTime() {
    if (timeClient.update()) {
        lastUpdateTime = getFormattedTime();
        Serial.printf("Time updated: %s\n", lastUpdateTime.c_str());
    } else {
        Serial.println("Failed to update time from NTP server");
    }
}

String getFormattedTime() {
    // Get hours, minutes, seconds
    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    int seconds = timeClient.getSeconds();

    // Convert to 12-hour format
    String ampm = "AM";
    if (hours >= 12) {
        ampm = "PM";
        if (hours > 12) hours -= 12;
    }
    if (hours == 0) hours = 12;

    // Format as String
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d %s", hours, minutes, seconds, ampm.c_str());
    return String(timeStr);
}

String getLastUpdateTime() {
    return lastUpdateTime;
} 

void drawTimeStamp(int x, int y, uint16_t color) {
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(x, y);
    M5.Lcd.printf("Updated: %s", lastUpdateTime.c_str());
}