/*
  ST7755_Menu.h - Keyboard library for ST7755 display
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7755_Crypto_Selector.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

//////////////////////////////////////////////// Menu ///////////////////////////////////////////////

// Constructor for Keyboard
Price_Selector::Price_Selector(Adafruit_ST7735* display)
{
	tft = display;
	
	selected_index = 0;
	
	value = 0;
	
	value_to_add = 1;
}

// Performs the action of the currently selected key
void Price_Selector::addValue(){
	value = value + value_to_add < 999999999999 ? value + value_to_add : value;
}

void Price_Selector::subValue(){
	value = value - value_to_add >= 0 ? value - value_to_add : 0;
}

// Display the menu on the screen
void Price_Selector::display()
{
	redrawValue();
	redrawValueChange();
}

void Price_Selector::setValue(double val){
	value = val;
}

double Price_Selector::getValue(){
	return value;
}

void Price_Selector::increaseValueToAdd(){
	value_to_add = value_to_add*10 < 10000000000 ? value_to_add*10 : value_to_add;
}

void Price_Selector::decreaseValueToAdd(){
	value_to_add = value_to_add/10 >= 0.000001 ? value_to_add/10 : value_to_add;
}

void Price_Selector::clear(){
	tft->fillRect(0, 75, tft->width(), 40, BLACK);
	tft->setTextSize(2);
}

void Price_Selector::redrawValue(){
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

void Price_Selector::redrawValueChange(){
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
