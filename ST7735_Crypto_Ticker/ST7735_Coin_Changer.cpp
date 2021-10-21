/*
  ST7735_Coin_Changer.cpp - Coin changer interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Coin_Changer.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

// Constructor for Portfolio Editor
ST7735_Coin_Changer::ST7735_Coin_Changer(Adafruit_ST7735* display, COIN* coin_arr) {
	tft = display;
	stage = 0;
	coins = coin_arr;
}

// Display the coin changer interface on screen
void ST7735_Coin_Changer::display(){
	if (stage == 0){
	  tft -> fillRect(0, 0, tft->width(), tft->height() - 12, BLACK);
	  tft -> setTextColor(WHITE);
	  
	  int i = 0;
	  while(i < COIN_COUNT){
		tft -> fillRect(map(i % 4, 0, 3, 2, tft -> width() - 43), map(floor(i / 4), 0, 5, tft -> height() / 4, 
		  3 * tft -> height() / 4), 4, 7, coins[i].portfolio_colour);
		tft -> setCursor(map(i % 4, 0, 3, 9, tft -> width() - 36), map(floor(i / 4), 0, 5, tft -> height() / 4, 
		  3 * tft -> height() / 4));
		tft -> print(coins[i].coin_code);
		i++;
	  }

	  tft -> fillRect(map(current_changing_index % 4, 0, 3, 9, tft -> width() - 36) - 1, 
		map(floor(current_changing_index / 4), 0, 5, tft -> height() / 4, 
		  3 * tft -> height() / 4) - 1, 31, 9, GRAY);
	  tft -> setCursor(map(current_changing_index % 4, 0, 3, 9, tft -> width() - 36), 
		map(floor(current_changing_index / 4), 0, 5, tft -> height() / 4, 
		  3 * tft -> height() / 4));
	  tft -> setTextColor(BLACK);
	  tft -> print(coins[current_changing_index].coin_code);
	}
}