/*
  ST7735_Coin_Changer.h - Coin changer interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <String.h>  
#include "ST7735_Coin.h"

// ensure this library description is only included once
#ifndef ST7735_Coin_Changer_h
#define ST7735_Coin_Changer_h

class ST7735_Coin_Changer
{
	public:
		ST7735_Coin_Changer(Adafruit_ST7735*, COIN*);
		void display();
		COIN* coins;
		int stage;
		int current_changing_index;

	private:
		Adafruit_ST7735* tft;
};


#endif
