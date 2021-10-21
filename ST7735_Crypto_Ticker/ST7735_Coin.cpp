#include "ST7735_Coin.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

// Constructor for Coin with bitmap
COIN::COIN(char* code, char* id, const unsigned char* bm, uint16_t circle_col, 
		uint16_t bm_col, uint16_t portfolio_col) {
	coin_code = code;
	coin_id = id;
	bitmap = bm;
	circle_colour = circle_col;
	bm_colour = bm_col;
	portfolio_colour = portfolio_col;
	amount = 0;
	
	bitmap_present = 1;
}

// Constructor for coin without bitmap
COIN::COIN(char* code, char* id, uint16_t portfolio_col) {
	coin_code = code;
	coin_id = id;
	portfolio_colour = portfolio_col;
	amount = 0;
	
	bitmap_present = 0;
}
