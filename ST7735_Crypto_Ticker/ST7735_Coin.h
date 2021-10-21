/*
  ST7735_Menu.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Candle_Graph.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <String.h>  

// ensure this library description is only included once
#ifndef ST7735_Coin_h
#define ST7735_Coin_h

#define COIN_COUNT          20

// For storing coins
class COIN {
	public:
	  COIN(char*, char*, const unsigned char*, uint16_t, 
		uint16_t, uint16_t);
	  COIN(char*, char*, uint16_t);
	
	  char* coin_code;
	  char* coin_id;
	  double current_price;
	  double current_change;
	  double amount;
	  const unsigned char* bitmap;
	  Candle_Graph* candles;
	  uint16_t circle_colour;
	  uint16_t bm_colour;
	  uint16_t portfolio_colour;
	  int bitmap_present;
};

#endif
