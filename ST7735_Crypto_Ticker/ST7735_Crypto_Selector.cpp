/*
  ST7755_Crypto_Selector.h - Amount Selector library for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Crypto_Selector.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h> // Extra colours

// Constructor for Amount Selector
Price_Selector::Price_Selector(Adafruit_ST7735* display) {
	tft = display;
	
	selected_index = 0;
	value = 0;
	value_to_add = 1;
}

// Adds the currently selected value
void Price_Selector::addValue() {
	value = value + value_to_add < 999999999999 ? 
		value + value_to_add : value;
}

// Subtracts the currently selected value
void Price_Selector::subValue() {
	value = value - value_to_add >= 0 ? value - value_to_add : 0;
}

// Display the selector on the screen
void Price_Selector::display() {
	redrawValue();
	redrawValueChange();
}

// Sets the value to the passed value
void Price_Selector::setValue(double val) {
	value = val;
}

// Returns the current value
double Price_Selector::getValue() {
	return value;
}

// Multiplies the selected value to add by 10
void Price_Selector::increaseValueToAdd() {
	value_to_add = value_to_add * 10 < 10000000000 ? 
		value_to_add * 10 : value_to_add;
}

// Divides the selected value to add by 10
void Price_Selector::decreaseValueToAdd() {
	value_to_add = value_to_add / 10 >= 0.000001 ? 
		value_to_add / 10 : value_to_add;
}

// Clears the area occupied by the selector
void Price_Selector::clear() {
	tft->fillRect(0, 75, tft->width(), 40, BLACK);
	tft->setTextSize(2);
}

// Redraws the value selected by the user
void Price_Selector::redrawValue() {
	tft->fillRect(0, 75, tft->width(), 15, BLACK);
	tft->setTextColor(WHITE);
	tft->setCursor(2, 75);
	tft->setTextSize(2);
	
	if (value >= 1000000){
		tft->print(value, 0);
	} else {
		tft->print(value, 6);
	}
}

// Redraws the value change selected by the user
void Price_Selector::redrawValueChange() {
	tft->setTextSize(1);
	tft->fillRect(0, 100, tft->width(), 8, BLACK);
	tft->setCursor(2, 100);
	tft->setTextColor(GREEN);
	tft->print('+');
	tft->setTextColor(RED);
	tft->print('-');
	tft->setTextColor(WHITE);
	tft->print(value_to_add, 6);
}
