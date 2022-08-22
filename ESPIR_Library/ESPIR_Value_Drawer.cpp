#include "ESPIR_Value_Drawer.h"

#define max(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })

ESPIR_Value_Drawer::ESPIR_Value_Drawer(Adafruit_ST7735* tft) {
	display = tft;
}

void ESPIR_Value_Drawer::drawPrice(double available_space, double price, double max_precision, int size, int currency){
    display->setTextSize(size);
    display->setTextColor(WHITE);

    if (currency == 0){ // GBP
        display->print(char(156));
    } else if (currency == 1){ // USD
        display->print(char(36));
    } else if (currency == 2){ // EUR
        display->print(char(237));
    }

    // Calculate the minimum and maximum values that can fit in the space
    double max = pow(10, available_space) - 1;
    double min = (pow(0.1, available_space - 2) - 5*pow(0.1, available_space) )* 10; // * 10 for precision of at least 2

    // Represent number normally, but limit occupied space
    if (price == 0){
        display->print("0.");

        for (int i = 0; i < available_space-2; i++){
            display->print('0');
        }
    } else if (price < min){
        double mul = 10;
        int e_val = -1;

        while (price*mul < pow(10, available_space - 4)){
            mul *= 10;
            e_val--;
        }

        if ((int) round(price*mul) == (int) pow(10, available_space - 3)){
            mul/=10;
            e_val++;
        }

        display->print((int) round(price*mul));
        display->print('e');
        display->print(e_val);
    } else if (price >= max) {
        double div = 10;
        int e_val = 1;

        while (price/div >= pow(10, available_space - 2)){
            div *= 10;
            e_val++;
        }

        if ((int) round(price/div) == (int) pow(10, available_space - 2)){
            div*=10;
            e_val++;
        }

        display->print((int) round(price/div));
        display->print('e');
        display->print(e_val);
    } else {
        int precision;

        if (floor(price) > 0){
            int integer_digits = (int) log10(floor(price)) + 1;
            precision = min(max_precision, available_space - integer_digits - 1);
        } else {
            precision = min(max_precision, available_space-2);
        }

        precision = max(0, precision); // Ensure no negative precision

        // Account for rounding increasing the size of the value
        if (floor(price) > 0){
            double test = round(price * pow(10, precision))/pow(10, precision);

            // If more digits after rounding by desired precision, call again on rounded
            if ((int) log10(floor(test)) + 1 > (int) log10(floor(price)) + 1){
                drawPrice(available_space, test, max_precision, size, currency);
                return;
            }
        }

        display->print(price, precision);
    }

    display->setTextColor(WHITE);
}

void ESPIR_Value_Drawer::drawSign(double value) {
    if (value < 0) {
        display->setTextColor(LIGHT_RED);
        display->print('-');
    } else {
        display->setTextColor(LIGHT_GREEN);
        display->print('+');
    }
}

// Draws the percentage change on the screen.
void ESPIR_Value_Drawer::drawPercentageChange(double available_space, 
        double value, double max_precision, int size) {
    display->setTextSize(1);

    int sign;

    double change_val;
    if (value < 0){
        sign = -1;
        change_val = -value;
    } else {
        sign = 1;
        change_val = value;
    }

    // Calculate the minimum and maximum values that can fit in the space
    double max = pow(10, available_space) - 1;
    double min = pow(0.1, available_space - 2) - 5*pow(0.1, available_space);

    // Represent number normally, but limit occupied space
    if (change_val == 0){
        drawSign(value);

        display->print("0.");

        for (int i = 0; i < available_space-2; i++){
            display->print('0');
        }

        display->print('%');
    } else if (change_val < min){
        double mul = 10;
        int e_val = -1;

        while (change_val*mul < pow(10, available_space - 4)){
            mul *= 10;
            e_val--;
        }

        if ((int) round(change_val*mul) == (int) pow(10, available_space - 3)){
            mul/=10;
            e_val++;
        }

        drawSign(value);
        display->print((int) round(change_val*mul));
        display->print('e');
        display->print(e_val);
        display->print('%');
    } else if (change_val >= max) {
        double div = 10;
        int e_val = 1;

        while (change_val/div >= pow(10, available_space - 2)){
            div *= 10;
            e_val++;
        }

        if ((int) round(change_val/div) == (int) pow(10, available_space - 2)){
            div*=10;
            e_val++;
        }

        drawSign(value);
        display->print((int) round(change_val/div));
        display->print('e');
        display->print(e_val);
        display->print('%');
    } else {
       int precision;

        if (floor(change_val) > 0){
            int integer_digits = (int) log10(floor(change_val)) + 1;

            precision = min(max_precision, available_space - integer_digits - 1);
        } else {
            precision = min(max_precision, available_space-2); // For the 0.
        }

        precision = max(0, precision);

        // Account for rounding
        if (floor(change_val) > 0){
            double test = round(change_val * pow(10, precision))/pow(10, precision);

            // If more digits after rounding by desired precision, call again on rounded
            if ((int) log10(floor(test)) + 1 > (int) log10(floor(change_val)) + 1){
                drawPercentageChange(available_space, sign * test, max_precision, size);
                return;
            }
        }

        drawSign(value);

        display->print(change_val, precision);
        
        display->print('%');
    }

    display->setTextColor(WHITE);
}