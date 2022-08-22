#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <cmath>

#include "Colours.h"

#ifndef ESPIR_Value_Drawer_h
#define ESPIR_Value_Drawer_h

class ESPIR_Value_Drawer {
    public:
        ESPIR_Value_Drawer(Adafruit_ST7735*);
        void drawPercentageChange(double, double, double, int);
        void drawPrice(double, double, double, int, int);
        void drawSign(double);

    private:
        Adafruit_ST7735* display;
};

#endif