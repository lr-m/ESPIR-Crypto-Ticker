/*
  ST7735_Menu.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <String.h>  

// ensure this library description is only included once
#ifndef ST7735_Portfolio_Editor_h
#define ST7735_Coin_h

#define COIN_COUNT          20

// For candle graph element
typedef struct graph_candle {
  float opening;
  float closing;
  float high;
  float low;
} G_CANDLE;

// For storing coins
class COIN {
	public:
	  char* coin_code;
	  char* coin_id;
	  double current_price;
	  double current_change;
	  double amount;
	  const unsigned char* bitmap;
	  G_CANDLE* candles;
	  uint16_t circle_colour;
	  uint16_t bm_colour;
	  uint16_t portfolio_colour;
};

#endif
