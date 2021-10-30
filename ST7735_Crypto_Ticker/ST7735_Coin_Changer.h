/*
  ST7735_Coin_Changer.h - Coin changer interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <String.h>  
#include "ST7735_Coin.h"
#include <ST7735_Keyboard.h>

// ensure this library description is only included once
#ifndef ST7735_Coin_Changer_h
#define ST7735_Coin_Changer_h

class Colour_Picker_Component
{
	public:
		Colour_Picker_Component(Adafruit_ST7735*, int, int, int);
		void display();
		void displaySelected();
		int x;
		int y;
		int colour;
		unsigned char value;
		void incrementValue();
		void decrementValue();
		unsigned char getValue();

	private:
		Adafruit_ST7735* tft;
};

class ST7735_Coin_Changer
{
	public:
		ST7735_Coin_Changer(Adafruit_ST7735*, char**, COIN*, ST7735_Keyboard*, const unsigned char*);
		void display();
		COIN* coins;
		const unsigned char* default_bitmap;
		int stage;
		int current_replacing_index;
		int rgb_to_bgr(unsigned char, unsigned char, unsigned char);
		int isActive();
		void setActive();
		void exit();
		int interact(uint32_t*);
		int active;
		ST7735_Keyboard* keyboard;
		char* loaded_id;
		char* loaded_code;
		int verified_id;
		void verificationSuccess();
		void verificationFailed();
		Colour_Picker_Component* pickers;
		int selected_picker;
		void drawColour();
		void loadIntoSelectedCoin();
		char** coin_code_list;
		void drawCoinSelected(int);
		void drawCoinUnselected(int);

	private:
		Adafruit_ST7735* tft;
};




#endif
