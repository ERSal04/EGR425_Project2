#include <M5Core2.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
// #include "EGR425_Phase1_weather_bitmap_images.h"
#include "WiFi.h"
#include "config.h"
#include "timestamp.h"
#include "layout.h"

// ===============================================
// THIS IS THE IN CLASS LAB FILE
// ===============================================

////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////
// TODO 3: Register for openweather account and get API key
String urlOpenWeather = "https://api.openweathermap.org/data/2.5/weather?";
String apiKey = OPENWEATHER_API_KEY;

// TODO 1: WiFi variables
String wifiNetworkName = "CBU-LANCERS";
String wifiPassword = "L@ncerN@tion";

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

// LCD variables
// int sWidth;
// int sHeight;

////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////
String httpGETRequest(const char *serverName);
// void drawWeatherImage(String iconId, int resizeMult);
String getZipcodeString();
void displayZipcodeInput();
void handleZipcodeInput();

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

///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop()
{
    M5.update();

    // Handle zipcode input mode
    if (zipcodeMode)
    {
        handleZipcodeInput();

        return; // Don't execute weather code yet
    }

    if (M5.BtnB.wasPressed()) {
                if (tempUnit == "imperial") {
                    tempUnit = "metric";
                    tempChar = "C";
                    Serial.printf("Changing units to Metric. Char: %s", tempChar.c_str());
                } else {
                    tempUnit = "imperial";
                    tempChar = "F";
                    Serial.printf("Changing units to Imperial. Char: %s", tempChar.c_str());
                }
                lastTime = 0;
            }

    if (M5.BtnC.wasPressed()) {
        // Pre-populate the digit array with the last used zipcode
        for (int i = 0; i < 5; i++) {
            zipcode[i] = lastZipCode[i] - '0'; // Convert char to int
        }
        currentDigit = 0;
        zipcodeMode = true;
        displayZipcodeInput();
    }

    // Only execute every so often
    if ((millis() - lastTime) > timerDelay)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            updateTime();

            //////////////////////////////////////////////////////////////////
            // TODO 4: Hardcode the specific city,state,country into the query
            // Examples: https://api.openweathermap.org/data/2.5/weather?q=riverside,ca,usa&units=imperial&appid=YOUR_API_KEY
            // https://api.openweathermap.org/data/2.5/weather?q=london&units=imperial&appid=YOUR_API_KEY
            // https://api.openweathermap.org/data/2.5/weather?q=chino,ca,usa&units=imperial&appid=YOUR_API_KEY
            // https://api.openweathermap.org/data/2.5/weather?q=washington,dc,usa&units=imperial&appid=YOUR_API_KEY
            // https://api.openweathermap.org/data/2.5/weather?q=tokyo,jp&units=imperial&appid=YOUR_API_KEY
            //////////////////////////////////////////////////////////////////
            // String serverURL = urlOpenWeather + "q=tokyo,jp&units=imperial&appId=" + apiKey;
            String serverURL = urlOpenWeather + "zip=" + zipcodeString + ",us&units=" + tempUnit + "&appId=" + apiKey;
            Serial.println(serverURL); // Debug print

            //////////////////////////////////////////////////////////////////
            // TODO 5: Make GET request and store reponse
            //////////////////////////////////////////////////////////////////
            String response = httpGETRequest(serverURL.c_str());
            Serial.print(response); // Debug print

            //////////////////////////////////////////////////////////////////
            // TODO 6: Import ArduinoLibrary and then use arduinojson.org/v6/assistant to
            // compute the proper capacity (this is a weird library thing) and initialize
            // the json object
            //////////////////////////////////////////////////////////////////
            JsonDocument objResponse;

            //////////////////////////////////////////////////////////////////
            // TODO 7: (uncomment) Deserialize the JSON document and test if parsing succeeded
            //////////////////////////////////////////////////////////////////
            DeserializationError error = deserializeJson(objResponse, response);
            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }
            serializeJsonPretty(objResponse, Serial); // Debug print

            //////////////////////////////////////////////////////////////////
            // TODO 8: Parse Response to get the weather description and icon
            //////////////////////////////////////////////////////////////////
            String strWeatherDesc = objResponse["weather"][0]["description"].as<String>();
            String strWeatherIcon = objResponse["weather"][0]["icon"].as<String>();
            String cityName = objResponse["name"].as<String>();

            // TODO 9: Parse response to get the temperatures
            double tempNow = objResponse["main"]["temp"].as<double>();
            double tempMin = objResponse["main"]["temp_min"].as<double>();
            double tempMax = objResponse["main"]["temp_max"].as<double>();

            Serial.printf("NOW: %.1f F and %s\tMIN: %.1f F\tMax: %.1f F\n", tempNow, strWeatherDesc, tempMin, tempMax);

            //////////////////////////////////////////////////////////////////
            // TODO 10: We can download the image directly, but there is no easy
            // way to work with a PNG file on the ESP32 (in Arduino) so we will
            // take another route - see EGR425_Phase1_weather_bitmap_images.h
            // for more details
            //////////////////////////////////////////////////////////////////
            String imagePath = "http://openweathermap.org/img/wn/" + strWeatherIcon + "@2x.png";
            Serial.println(imagePath);
            response = httpGETRequest(imagePath.c_str());
            Serial.print(response); 

            uint16_t primaryTextColor;
            // Draws the Weather details with a gradient and prints the last time it was updated
            drawGradientStyle(tempNow, tempMin, tempMax, cityName, strWeatherDesc, strWeatherIcon, primaryTextColor, tempChar);

        }
        else
        {
            Serial.println("WiFi Disconnected");
        }

        // Update the last time to NOW
        lastTime = millis();
    }
    // delay(100);
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

/////////////////////////////////////////////////////////////////
// This method takes in an image icon string (from API) and a
// resize multiple and draws the corresponding image (bitmap byte
// arrays found in EGR425_Phase1_weather_bitmap_images.h) to scale (for
// example, if resizeMult==2, will draw the image as 200x200 instead
// of the native 100x100 pixels) on the right-hand side of the
// screen (centered vertically).
/////////////////////////////////////////////////////////////////
// void drawWeatherImage(String iconId, int resizeMult)
// {

//     // Get the corresponding byte array
//     const uint16_t *weatherBitmap = getWeatherBitmap(iconId);

//     // Compute offsets so that the image is centered vertically and is
//     // right-aligned
//     int yOffset = -(resizeMult * imgSqDim - M5.Lcd.height()) / 2;
//     int xOffset = sWidth - (imgSqDim * resizeMult * .8); // Right align (image doesn't take up entire array)
//     // int xOffset = (M5.Lcd.width() / 2) - (imgSqDim * resizeMult / 2); // center horizontally

//     // Iterate through each pixel of the imgSqDim x imgSqDim (100 x 100) array
//     for (int y = 0; y < imgSqDim; y++)
//     {
//         for (int x = 0; x < imgSqDim; x++)
//         {
//             // Compute the linear index in the array and get pixel value
//             int pixNum = (y * imgSqDim) + x;
//             uint16_t pixel = weatherBitmap[pixNum];

//             // If the pixel is black, do NOT draw (treat it as transparent);
//             // otherwise, draw the value
//             if (pixel != 0)
//             {
//                 // 16-bit RBG565 values give the high 5 pixels to red, the middle
//                 // 6 pixels to green and the low 5 pixels to blue as described
//                 // here: http://www.barth-dev.de/online/rgb565-color-picker/
//                 byte red = (pixel >> 11) & 0b0000000000011111;
//                 red = red << 3;
//                 byte green = (pixel >> 5) & 0b0000000000111111;
//                 green = green << 2;
//                 byte blue = pixel & 0b0000000000011111;
//                 blue = blue << 3;

//                 // Scale image; for example, if resizeMult == 2, draw a 2x2
//                 // filled square for each original pixel
//                 for (int i = 0; i < resizeMult; i++)
//                 {
//                     for (int j = 0; j < resizeMult; j++)
//                     {
//                         int xDraw = x * resizeMult + i + xOffset;
//                         int yDraw = y * resizeMult + j + yOffset;
//                         M5.Lcd.drawPixel(xDraw, yDraw, M5.Lcd.color565(red, green, blue));
//                     }
//                 }
//             }
//         }
//     }
// }
