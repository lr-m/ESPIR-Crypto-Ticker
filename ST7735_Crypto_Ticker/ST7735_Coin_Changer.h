/*
  ST7735_Coin_Changer.h - Coin changer interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "Colours.h"
#include "IR_Codes.h"
#include "ST7735_Keyboard.h"
#include "ST7735_Coin.h"

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
		void incrementValue(int);
		unsigned char getValue();
		void clear();

	private:
		Adafruit_ST7735* tft;
};

class ST7735_Coin_Changer
{
	public:
		ST7735_Coin_Changer(Adafruit_ST7735*, char**, COIN*, ST7735_Keyboard*);
		void display();
		COIN* coins;
		int stage;
		int current_replacing_index;
		int rgb_to_bgr(unsigned char, unsigned char, unsigned char);
		int isActive();
		void setActive();
		void exit();
		int interact(uint32_t*);
		int active;
		ST7735_Keyboard* keyboard;
		char loaded_id[30];
		char loaded_code[10];
		int verified_id;
		
		void verificationSuccess();
		void verificationFailed();
		void duplicateDetected();
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
