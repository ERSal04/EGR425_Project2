#include "layout.h"

int sHeight;
int sWidth;

void getScreenMetrics(){
    sWidth = M5.Lcd.width();
    sHeight = M5.Lcd.height();
}

void drawGradientStyle(double tempNow, double tempMin, double tempMax, String cityName, 
                    String weatherDesc, String iconId, uint16_t primaryTextColor, String tempChar) {

    getScreenMetrics();
    
    for (int y = 0; y < sHeight; y++) {
        uint16_t color;
        if (iconId.indexOf('d') > 0) {
            int blue = map(y, 0, sHeight, 31, 15);
            color = M5.Lcd.color565(135, 206, blue * 8);
        } else {
            int brightness = map(y, 0, sHeight, 60, 0);
            color = M5.Lcd.color565(brightness/4, brightness/4, brightness);
        }
        M5.Lcd.drawFastHLine(0, y, sWidth, color);
    }

    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(30, 40);
    M5.Lcd.print(cityName);

    M5.Lcd.setTextSize(10);
    M5.Lcd.setCursor(30, 60);
    M5.Lcd.printf("%.0f%s", tempNow, tempChar.c_str());

    drawWeatherImage(iconId, 2);

    M5.Lcd.fillRoundRect(15, sHeight - 70, sWidth - 30, 55, 10, TFT_BLACK);
    M5.Lcd.drawRoundRect(15, sHeight - 70, sWidth - 30, 55, 10, TFT_WHITE);
    
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_CYAN);
    M5.Lcd.setCursor(30, sHeight - 60);
    M5.Lcd.printf("%.0f%s", tempMin, tempChar.c_str());
    
    M5.Lcd.setTextColor(TFT_ORANGE);
    M5.Lcd.setCursor(sWidth - 80, sHeight - 60);
    M5.Lcd.printf("%.0f%s", tempMax, tempChar.c_str());
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(30, sHeight - 35);
    M5.Lcd.print(weatherDesc);
    
    drawTimeStamp(sWidth/2 + 10, sHeight - 35, TFT_LIGHTGREY);
}

void drawWeatherImage(String iconId, int resizeMult)
{

    // Get the corresponding byte array
    const uint16_t *weatherBitmap = getWeatherBitmap(iconId);

    // Compute offsets so that the image is centered vertically and is
    // right-aligned
    int yOffset = -(resizeMult * imgSqDim - M5.Lcd.height()) / 2 - 40;
    int xOffset = sWidth - (imgSqDim * resizeMult * .8); // Right align (image doesn't take up entire array)
    // int xOffset = (M5.Lcd.width() / 2) - (imgSqDim * resizeMult / 2); // center horizontally

    // Iterate through each pixel of the imgSqDim x imgSqDim (100 x 100) array
    for (int y = 0; y < imgSqDim; y++)
    {
        for (int x = 0; x < imgSqDim; x++)
        {
            // Compute the linear index in the array and get pixel value
            int pixNum = (y * imgSqDim) + x;
            uint16_t pixel = weatherBitmap[pixNum];

            // If the pixel is black, do NOT draw (treat it as transparent);
            // otherwise, draw the value
            if (pixel != 0)
            {
                // 16-bit RBG565 values give the high 5 pixels to red, the middle
                // 6 pixels to green and the low 5 pixels to blue as described
                // here: http://www.barth-dev.de/online/rgb565-color-picker/
                byte red = (pixel >> 11) & 0b0000000000011111;
                red = red << 3;
                byte green = (pixel >> 5) & 0b0000000000111111;
                green = green << 2;
                byte blue = pixel & 0b0000000000011111;
                blue = blue << 3;

                // Scale image; for example, if resizeMult == 2, draw a 2x2
                // filled square for each original pixel
                for (int i = 0; i < resizeMult; i++)
                {
                    for (int j = 0; j < resizeMult; j++)
                    {
                        int xDraw = x * resizeMult + i + xOffset;
                        int yDraw = y * resizeMult + j + yOffset;
                        M5.Lcd.drawPixel(xDraw, yDraw, M5.Lcd.color565(red, green, blue));
                    }
                }
            }
        }
    }
}