/*
  ST7735_Crypto_Selector.h - Amount Selector library for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

// ensure this library description is only included once
#ifndef ST7735_Crypto_Selector_h
#define ST7735_Crypto_Selector_h

class Price_Selector
{
	public:
		Price_Selector(Adafruit_ST7735*);
		void display();
		void moveLeft();
		void moveRight();
		void press();
		void setValue(double);
		double getValue();
		void increaseValueToAdd();
		void decreaseValueToAdd();
		void clear();
		void addValue();
		void subValue();
		void redrawValue();
		void redrawValueChange();

		int current_changing_index;

	private:
		Adafruit_ST7735* tft;
		int selected_index;
		int in_selector;
		double value;
		double value_to_add;
};

#endif
