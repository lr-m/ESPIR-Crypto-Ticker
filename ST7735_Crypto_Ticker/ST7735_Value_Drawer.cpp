#include "ST7735_Value_Drawer.h"
#include <cmath>

 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
 
 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

ST7735_Value_Drawer::ST7735_Value_Drawer(Adafruit_ST7735* tft)
{
	display = tft;
}

void ST7735_Value_Drawer::drawPrice(double available_space, double price, double max_precision, int size, int currency){
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

    int counter = 0; // Counts current number of digits
    int int_buffer = 0; // Buffer for the value before decimal point
    int dec_buffer = 0; // Buffer for the value after decimal point

    int dec_zeros = 0;
    int decimal_comp_size = 0;

    bool has_decimal_component = false;

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
        if (floor(price) > 0){
            // Print integers if there are any
            int integer_digits = (int) log10(floor(price))+1;

            // If decimal values won't be drawn, then round the value
            if (integer_digits >= available_space-1){
                price = round(price);
                integer_digits = (int) log10(floor(price))+1;

                if ((int) log10(floor(price)) > integer_digits){
                    drawPrice(available_space, price, max_precision, size, currency);
                    return;
                }

                display->print(price);
                return;
            } else {
                // Draw the integer digits
                for (int i = 1; i <= integer_digits; i++){
                    int_buffer *= 10;
                    int_buffer += fmod(floor(floor(price)/pow(10, integer_digits - i)), 10);
                    counter++;
                }

                // Increment counter to accomodate the decimal point
                if (counter < available_space-1){
                    counter++;
                    has_decimal_component = true;
                }
            }
        } else if (floor(price) == 0){
            // If no integer digits, < 0
            int_buffer = 0;
            counter+=2; // Accomodates the 0. part
            has_decimal_component = true;
        }

        int size_cap = counter;

        // Print the decimal component
        if (fmod(price, 1) != 0){
            // Get integer representation of the decimal component
            decimal_comp_size = min(max_precision, available_space - counter);
            size_cap += decimal_comp_size;

            double decimal_comp = (price - floor(price)) * pow(10, decimal_comp_size);

            // If rounding the decimal component results in more digits (i.e. 9.99 -> 10.0), handle
            if (!std::isinf(log10((int) floor(decimal_comp))) && !std::isinf(log10((int) round(decimal_comp))) &&
                    floor(log10((int) floor(decimal_comp))) < floor(log10((int) round(decimal_comp))) && !(floor(log10((int) floor(decimal_comp))) == 0 && round(log10((int) round(decimal_comp))) == 1)){
                int_buffer+=1;
                decimal_comp = 0;

                if ((int) log10(floor(price)) < (int) log10(floor((double) int_buffer))){
                    drawPrice(available_space, (double) int_buffer, max_precision, size, currency);
                    return;
                }
            } else if (counter <= available_space-1) {
                // Print like integer in the remaining space
                for (int i = 1; i <= decimal_comp_size; i++){
                    dec_buffer *= 10;
                    dec_buffer += fmod(floor(round(decimal_comp)/pow(10, decimal_comp_size-i)), 10);
                    counter++;

                    if (dec_buffer == 0 && dec_zeros < max_precision){
                        dec_zeros++;
                    }
                }
            }
        } else if (has_decimal_component){
            for (int i = counter; i < available_space; i++){
                dec_zeros++;
            }
        }

        display->print(int_buffer);
        display->print('.');

        if (dec_buffer == 0){
            counter++;
        }

        if (dec_zeros != 0){
            for (int i = 0; i < dec_zeros; i++){
                display->print('0');
            }

            if (dec_buffer != 0){
                display->print(dec_buffer);
            }
        } else {
            display->print(dec_buffer);
        }

        // Fix if extra zeros needed 
        for (int i = counter; i < size_cap; i++){
            display->print('0');
        }
    }

    display->setTextColor(WHITE);
}

void ST7735_Value_Drawer::drawSign(double value){
    if (value < 0) {
        display->setTextColor(RED);
        display->print('-');
    } else {
        display->setTextColor(ST77XX_GREEN);
        display->print('+');
    }
}

// Draws the percentage change on the screen.
void ST7735_Value_Drawer::drawPercentageChange(double available_space, double value, double max_precision, int size) {
  display->setTextSize(1);

  double change_val;
  if (value < 0){
  	change_val = -value;
  } else {
  	change_val = value;
  }

  // Calculate the minimum and maximum values that can fit in the space
    double max = pow(10, available_space) - 1;
    double min = pow(0.1, available_space - 2) - 5*pow(0.1, available_space);

    int counter = 0; // Counts current number of digits
    int int_buffer = 0; // Buffer for the value before decimal point
    int dec_buffer = 0; // Buffer for the value after decimal point

    int dec_zeros = 0;

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
        if (floor(change_val) > 0){
            // Print integers if there are any
            int integer_digits = (int) log10(floor(change_val))+1;

            // If decimal values won't be drawn, then round the value
            if (integer_digits >= available_space-1){
                change_val = round(change_val);
                integer_digits = (int) log10(floor(change_val))+1;

                if ((int) log10(floor(change_val)) > integer_digits){
                    drawPercentageChange(available_space, change_val, max_precision, size);
                    return;
                }

                drawSign(value);
                display->print(change_val);
                display->print('%');
                return;
            } else {
                // Draw the integer digits
                for (int i = 1; i <= integer_digits; i++){
                    int_buffer *= 10;
                    int_buffer += fmod(floor(floor(change_val)/pow(10, integer_digits - i)), 10);
                    counter++;
                }

                // Increment counter to accomodate the decimal point
                if (counter < available_space-1){
                    counter++;
                }
            }
        } else if (floor(change_val) == 0){
            // If no integer digits, < 0
            int_buffer = 0;
            counter+=2; // Accomodates the 0. part
        }

        // Print the decimal component
        if (fmod(change_val, 1) != 0){
            // Get integer representation of the decimal component
            int size = min(max_precision, available_space - counter);
            double decimal_comp = (change_val - floor(change_val)) * pow(10, size);

            // If rounding the decimal component results in more digits (i.e. 9.99 -> 10.0), handle
            if (!std::isinf(log10((int) floor(decimal_comp))) && !std::isinf(log10((int) round(decimal_comp))) &&
                    floor(log10((int) floor(decimal_comp))) < floor(log10((int) round(decimal_comp))) && !(floor(log10((int) floor(decimal_comp))) == 0 && round(log10((int) round(decimal_comp))) == 1)){
                int_buffer+=1;
                decimal_comp = 0;

                if ((int) log10(floor(change_val)) < (int) log10(floor((double) int_buffer))){

                    drawPercentageChange(available_space, (double) int_buffer, max_precision, size);
                    return;
                }
            } else if (counter <= available_space-1) {
                // Print like integer in the remaining space
                for (int i = 1; i <= size; i++){
                    dec_buffer *= 10;
                    dec_buffer += fmod(floor(round(decimal_comp)/pow(10, size-i)), 10);
                    counter++;

                    if (dec_buffer == 0 && dec_zeros < max_precision){
                        dec_zeros++;
                    }
                }
            }
        }

        drawSign(value);
        display->print(int_buffer);
        display->print('.');

        if (dec_zeros != 0){
            for (int i = 0; i < dec_zeros; i++){
                display->print('0');
            }

            if (dec_buffer != 0){
                display->print(dec_buffer);
            }
        } else {
            display->print(dec_buffer);
        }

        for (int i = counter; i < available_space-1; i++){
            display->print('0');
        }
        
        display->print('%');
    }

    display->setTextColor(WHITE);
}