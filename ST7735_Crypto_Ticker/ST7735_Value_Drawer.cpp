#include "ST7735_Value_Drawer.h"

ST7735_Value_Drawer::ST7735_Value_Drawer(Adafruit_ST7735* tft)
{
	display = tft;
}

void ST7735_Value_Drawer::drawPrice(double price, int space, int size, int currency, int decimal_places, int cut_flag){
  // Get minimum and maximum possible display values
  double maximum = pow(10, space);
  double minimum = 1/pow(10, space-3);

  Serial.println(price);

  display->setTextSize(size);
  display->setTextColor(WHITE);

  if (currency == 0){ // GBP
    display->print(char(156));
  } else if (currency == 1){ // USD
    display->print(char(36));
  } else if (currency == 2){ // EUR
    display->print(char(237));
  }

  if (price == 0){
    display->print("0.00");
  } else if (price > minimum && price < maximum){
      int div = 1;
      int digits = 0;

      // Get number of integers
      while(price/div >= 1){
          digits++;
          div *= 10;
      }

      // Populate the to_display array with the integer digits
      if (digits > 0){
          int i = 0;
          while(price/div < price){
              div/=10;
              display->print((int) std::fmod(price/div, 10));
              i++;        
          }

          // Add decimal point
          if (digits < space - 1){
              display->print('.');       
          }
      } else {
          display->print("0.");
          digits ++;
      }

      // Fill in space after decimal point
      int mult = 10;

      int iter_val = space;
      if (cut_flag == 1) {
        iter_val = min(digits + decimal_places + 1, space);
      }

      for (int i = digits + 1; i < iter_val; i++){
        if (i == digits + decimal_places){
            display->print((int) std::fmod(round(price*mult), 10));
        } else if (i < digits + decimal_places) {
            display->print((int) std::fmod(price*mult, 10));
        } else {
            display->print('0');
        }
        mult *= 10;
      }
  } else if (price >= maximum) {
      long int div = 10;
      int e_val = 1;

      while (price/div >= pow(10, space-2)){
          div*=10;
          e_val++;
      }

      display->print((int) round(price/div));
      display->print('e');
      display->print(e_val);
  } else if (price <= minimum){
      long int mul = 10;
      int e_val = -1;

      while (price*mul < pow(10, space-4)){
          mul *= 10;
          e_val--;
      }

      if (e_val <= -10){
          e_val++;
          mul/=10;
      }

      display->print((int) round(price*mul));
      display->print('e');
      display->print(e_val);
  }
}

// Draws the percentage change on the screen.
void ST7735_Value_Drawer::drawPercentageChange(double change, int space, int size) {
  display->setTextSize(size);
  
  if (change < 0) {
    display->setTextColor(RED);
    display->print('-');
  } else {
    display->setTextColor(ST77XX_GREEN);
    display->print('+');
  }

  double change_val = sqrt(change*change);
  Serial.println(change);
  Serial.println(change_val);

  // Get minimum and maximum possible display values
  double maximum = pow(10, space);
  double minimum = 1/pow(10, space-3);

  if ((int) (change_val * 100000) == 0){
    display->print('0');
  } else if (change_val > minimum && change_val < maximum){
      int div = 1;
      int digits = 0;

      // Get number of integers
      while(change_val/div >= 1){
          digits++;
          div *= 10;
      }

      // Populate the to_display array with the integer digits
      if (digits > 0){
          int i = 0;
          while(change_val/div < change_val){
              div/=10;
              display->print((int) std::fmod(change_val/div, 10));
              i++;        
          }

          // Add decimal point
          if (digits < space-1){
              display->print('.');       
          }
          
      } else {
          display->print("0.");
          digits ++;
      }

      // Fill in space after decimal point
      int mult = 10;
      for (int i = digits + 1; i < space; i++){
          display->print((int) std::fmod(change_val*mult, 10));
          mult *= 10;
      }
  } else if (change_val >= maximum) {
      long int div = 10;
      int e_val = 1;

      while (change_val/div >= pow(10, space-2)){
          div*=10;
          e_val++;
      }

      display->print((int) round(change_val/div));
      display->print('e');
      display->print(e_val);
  } else if (change_val <= minimum){
      long int mul = 10;
      int e_val = -1;

      while (change_val*mul < pow(10, space-4)){
          mul *= 10;
          e_val--;
      }

      if (e_val <= -10){
          e_val++;
          mul/=10;
      }

      display->print((int) round(change_val*mul));
      display->print('e');
      display->print(e_val);
  }

  display->print('%');
  display->setTextColor(WHITE);
}