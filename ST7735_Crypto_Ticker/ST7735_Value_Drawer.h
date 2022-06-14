#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

#ifndef ST7735_Value_Drawer_h
#define ST7735_Value_Drawer_h

class ST7735_Value_Drawer
{
    public:
        ST7735_Value_Drawer(Adafruit_ST7735*);
        void drawPercentageChange(double, int, int);
        void drawPrice(double, int, int, int, int = 100, int = 0);

    private:
        Adafruit_ST7735* display;
};

#endif