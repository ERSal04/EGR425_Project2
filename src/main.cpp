// EGR 425 Project 2: Sensors and weather app
#include <M5Core2.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "config.h"
#include "timestamp.h"
#include "layout.h"
#include "SensorSuite.h"

////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////
// TODO 3: Register for openweather account and get API key
String urlOpenWeather = "https://api.openweathermap.org/data/2.5/weather?";
String apiKey = OPENWEATHER_API_KEY;

// TODO 1: WiFi variables
String wifiNetworkName = WIFI_SSID;
String wifiPassword = WIFI_PASSWORD;

// Time variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // 5000; 5 minutes (300,000ms) or 5 seconds (5,000ms)

// Zipcode input variables
int zipcode[5] = {9, 0, 2, 1, 0}; // Default zipcode (90210 as example)
int currentDigit = 0;             // Which digit we're currently editing (0-4)
bool zipcodeMode = true;          // Are we in zipcode input mode?
String zipcodeString = "90210";   // String version for API
String lastZipCode = "00000";

// Units Variable
String tempUnit = "imperial";
String tempChar = "F";
SensorSuite sensorSuite;
bool sensorsReady = false;

// Screen state
enum Screen { SCREEN_ZIPCODE, SCREEN_WEATHER, SCREEN_LOCAL };
Screen currentScreen = SCREEN_ZIPCODE;

// Local sensor update timing
unsigned long lastSensorTime = 0;
unsigned long sensorDelay = 5000;
float lastDisplayedTemp = -999;
float lastDisplayedHumidity = -999;

// Proximity / brightness control
bool screenIsOn = true;
uint16_t PROXIMITY_THRESHOLD = 300; // tune this after testing
int currentBrightness = 200;

////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////
String httpGETRequest(const char *serverName);
String getZipcodeString();
void displayZipcodeInput();
void handleZipcodeInput();
void redrawCurrentScreen(); 
void setScreenBrightness(int brightness); 

///////////////////////////////////////////////////////////////
// Put your setup code here, to run once
///////////////////////////////////////////////////////////////
void setup()
{
    // Initialize the device
    M5.begin();
    
    // Show zipcode input IMMEDIATELY so user sees something
    displayZipcodeInput();

    getScreenMetrics();
    sensorsReady = sensorSuite.begin(true, false);

    // TODO 2: Connect to WiFi (this can take several seconds)
    WiFi.begin(wifiNetworkName.c_str(), wifiPassword.c_str());
    Serial.printf("Connecting...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\n\nConnected to Wi-Fi network with IP address: ");
    Serial.println(WiFi.localIP());

    initTimeClient();
    
    // Redraw after WiFi is connected (in case WiFi messages overlapped)
    displayZipcodeInput();
}

void updateScreenHardware(SensorReadings &r) {
    if (!sensorsReady || !r.valid) return;

    bool objectClose = (r.proximityRaw > PROXIMITY_THRESHOLD);

    if (objectClose && screenIsOn) {
        screenIsOn = false;
        setScreenBrightness(0);
        M5.Lcd.sleep();
        Serial.printf("Screen OFF (prox=%u)\n", r.proximityRaw);
    }
    else if (!objectClose && !screenIsOn) {
        screenIsOn = true;
        M5.Lcd.wakeup();
        setScreenBrightness(currentBrightness);
        redrawCurrentScreen();
        Serial.printf("Screen ON (prox=%u)\n", r.proximityRaw);
    }

    if (screenIsOn) {
        float lux = constrain(r.lightLux, 0.0f, 1000.0f);
        int targetBrightness = (int)map((long)lux, 0, 1000, 30, 200);

        if (abs(targetBrightness - currentBrightness) > 5) {
            currentBrightness = targetBrightness;
            setScreenBrightness(currentBrightness);
            Serial.printf("Brightness: %d (lux=%.1f)\n", currentBrightness, r.lightLux);
        }
    }
}

void redrawCurrentScreen() {
    if (currentScreen == SCREEN_ZIPCODE) {
        displayZipcodeInput();
    }
    else if (currentScreen == SCREEN_WEATHER) {
        lastTime = 0; // force immediate weather refresh
    }
    else if (currentScreen == SCREEN_LOCAL) {
        lastDisplayedTemp = -999; // force immediate sensor redraw
        lastSensorTime = 0;
    }
}

void setScreenBrightness(int brightness) {
    // Core2 backlight is controlled via AXP192 LCD voltage
    // Range: 2500mV (off/dim) to 3300mV (full bright)
    brightness = constrain(brightness, 0, 200);
    uint16_t voltage = map(brightness, 0, 200, 2500, 3300);
    M5.Axp.SetLcdVoltage(voltage);
    Serial.printf("LCD Voltage: %dmV (brightness=%d)\n", voltage, brightness);
}

///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop()
{
    M5.update();

    // ── SCREEN: ZIPCODE ──────────────────────────────────────
    if (currentScreen == SCREEN_ZIPCODE) {
        handleZipcodeInput(); // internally sets zipcodeMode=false when done
        if (!zipcodeMode) {
            currentScreen = SCREEN_WEATHER;
            lastTime = 0;
        }
        return;
    }

    static unsigned long lastSensorRead = 0;
    static SensorReadings r;

    if (millis() - lastSensorRead > 100) {
        lastSensorRead = millis();
        r = sensorSuite.read(false);
        updateScreenHardware(r);
    }


    // ── SCREEN: LOCAL SENSOR ─────────────────────────────────
    if (currentScreen == SCREEN_LOCAL) {
        // Btn A: back to weather
        if (M5.BtnA.wasPressed()) {
            currentScreen = SCREEN_WEATHER;
            lastTime = 0; // force weather redraw
            lastDisplayedTemp = -999;
            lastDisplayedHumidity = -999;
            return;
        }

        // Btn B: toggle units
        if (M5.BtnB.wasPressed()) {
            if (tempUnit == "imperial") { tempUnit = "metric";  tempChar = "C"; }
            else                        { tempUnit = "imperial"; tempChar = "F"; }
            lastDisplayedTemp = -999; // force redraw
        }

        // Update every 5s, only redraw if values changed
        if ((millis() - lastSensorTime) > sensorDelay) {
            lastSensorTime = millis();
            updateTime();

            float displayTemp = (tempChar == "F")
                ? (r.tempC * 9.0f / 5.0f + 32.0f)
                : r.tempC;

            // Only redraw if values changed meaningfully
            if (abs(displayTemp - lastDisplayedTemp) > 0.05f ||
                abs(r.humidity - lastDisplayedHumidity) > 0.1f) {
                lastDisplayedTemp     = displayTemp;
                lastDisplayedHumidity = r.humidity;
                drawLocalSensorScreen(r.tempC, r.humidity, tempChar, r.sht40Valid);
            }
        }
        return;
    }

    // ── SCREEN: WEATHER ───────────────────────────────────────
    // Btn A: go to local sensor screen
    if (M5.BtnA.wasPressed()) {
        currentScreen = SCREEN_LOCAL;
        lastSensorTime = 0; // force immediate sensor read
        lastDisplayedTemp = -999;
        return;
    }

    // Btn B: toggle units
    if (M5.BtnB.wasPressed()) {
        if (tempUnit == "imperial") { tempUnit = "metric";  tempChar = "C"; }
        else                        { tempUnit = "imperial"; tempChar = "F"; }
        lastTime = 0;
    }

    // Btn C: back to zipcode
    if (M5.BtnC.wasPressed()) {
        for (int i = 0; i < 5; i++) zipcode[i] = lastZipCode[i] - '0';
        currentDigit = 0;
        zipcodeMode = true;
        currentScreen = SCREEN_ZIPCODE;
        displayZipcodeInput();
        return;
    }

    // Weather fetch every timerDelay ms
    if ((millis() - lastTime) > timerDelay) {
        if (WiFi.status() == WL_CONNECTED) {
            updateTime();
            String serverURL = urlOpenWeather + "zip=" + zipcodeString + ",us&units=" + tempUnit + "&appId=" + apiKey;
            String response = httpGETRequest(serverURL.c_str());

            JsonDocument objResponse;
            DeserializationError error = deserializeJson(objResponse, response);
            if (error) {
                Serial.printf("deserializeJson() failed: %s\n", error.f_str());
                return;
            }

            String strWeatherDesc = objResponse["weather"][0]["description"].as<String>();
            String strWeatherIcon = objResponse["weather"][0]["icon"].as<String>();
            String cityName = objResponse["name"].as<String>();
            double tempNow = objResponse["main"]["temp"].as<double>();
            double tempMin = objResponse["main"]["temp_min"].as<double>();
            double tempMax = objResponse["main"]["temp_max"].as<double>();

            uint16_t primaryTextColor;
            drawGradientStyle(tempNow, tempMin, tempMax, cityName, strWeatherDesc, strWeatherIcon, primaryTextColor, tempChar);

            drawSensorPanel(sensorsReady && r.valid, r.proximityRaw, r.lightLux);
        } else {
            Serial.println("WiFi Disconnected");
        }
        lastTime = millis();
    }
}

/////////////////////////////////////////////////////////////////
// This method takes in a URL and makes a GET request to the
// URL, returning the response.
/////////////////////////////////////////////////////////////////
String httpGETRequest(const char *serverURL)
{

    // Initialize client
    HTTPClient http;
    http.begin(serverURL);

    // Send HTTP GET request and obtain response
    int httpResponseCode = http.GET();
    String response = http.getString();

    // Check if got an error
    if (httpResponseCode > 0)
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    else
    {
        Serial.printf("HTTP Response ERROR code: %d\n", httpResponseCode);
        Serial.printf("Server Response: %s\n", response);
    }

    // Free resources and return response
    http.end();
    return response;
}

///////////////////////////////////////////////////////////////////
//                      Get Based off area codes                 //
//===============================================================//
//    1. Ability to have 5 places to enter numbers               //
//    2. Ensure you can increment each individual place up to 9  //
//    3. Link area codes to locations                            //
//    4. Change locations based off output                       //
///////////////////////////////////////////////////////////////////

// Function to convert zipcode array to string
String getZipcodeString()
{
    String result = "";
    for (int i = 0; i < 5; i++)
    {
        result += String(zipcode[i]);
    }
    return result;
}

// Function to display zipcode input UI
void displayZipcodeInput()
{
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(50, 50);
    M5.Lcd.println("Enter Zipcode:");

    // Display each digit
    int xStart = 60;
    int yPos = 120;
    for (int i = 0; i < 5; i++)
    {
        // Highlight current digit
        if (i == currentDigit)
        {
            M5.Lcd.fillRect(xStart + (i * 40) - 5, yPos - 5, 35, 50, TFT_BLUE);
        }

        M5.Lcd.setTextColor(i == currentDigit ? TFT_YELLOW : TFT_WHITE);
        M5.Lcd.setCursor(xStart + (i * 40), yPos);
        M5.Lcd.setTextSize(5);
        M5.Lcd.print(zipcode[i]);
    }

    // Instructions
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.setCursor(20, 200);
    M5.Lcd.println("A: Prev  B: +1  C: Next");

    if (lastZipCode != "00000") {
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(TFT_DARKGREY);
        M5.Lcd.setCursor(20, 220);
        M5.Lcd.printf("Last: %s", lastZipCode.c_str());
    }
}

// Function to handle zipcode input
void handleZipcodeInput()
{
    if (M5.BtnA.wasPressed())
    {
        // Move to previous digit
        currentDigit--;
        if (currentDigit < 0)
            currentDigit = 4;
        displayZipcodeInput();
    }

    if (M5.BtnC.wasPressed())
    {
        // Move to next digit
        currentDigit++;
        if (currentDigit > 4)
        {
            // Done entering zipcode
            zipcodeMode = false;
            zipcodeString = getZipcodeString();
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setTextSize(3);
            M5.Lcd.setCursor(50, 100);
            M5.Lcd.setTextColor(TFT_GREEN);
            M5.Lcd.printf("Getting weather for\n     %s...", zipcodeString.c_str());
            delay(1500);
            lastTime = 0; // Force immediate weather update
            lastZipCode = zipcodeString;
        }
        else
        {
            displayZipcodeInput();
        }
    }

    if (M5.BtnB.wasPressed())
    {
        // Increment current digit
        zipcode[currentDigit]++;
        if (zipcode[currentDigit] > 9)
        {
            zipcode[currentDigit] = 0;
        }
        displayZipcodeInput();
    }
}