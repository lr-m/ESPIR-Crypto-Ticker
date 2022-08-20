/*
  ST7735_Keyboard.h - Keyboard for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "Colours.h"
#include "IR_Codes.h"

#ifndef ST7735_Keyboard_h
#define ST7735_Keyboard_h

#define KEY_HEIGHT 11
#define NORMAL_KEY_WIDTH 12
#define EDGE_BORDER 4

class Key {
	public:
		Key(Adafruit_ST7735*, int, int, int, int, char*, int);
		void display(int);
		void displaySelected(int);
		
		int x;
		int y;
		int w;
		int h;
		char* action;
		
	private:
		Adafruit_ST7735* tft;
		int bottom_key;
};

class ST7735_Keyboard {
  public:
    ST7735_Keyboard(Adafruit_ST7735*);
	void press();
	void displayCurrentString();
    void display();
	void displayPrompt(char* prompt);
	void moveRight();
	void moveLeft();
	void moveUp();
	void moveDown();
	void setMode(int);
	void setMode(int, int);
	void reset();
	void goToTabs();
	void exitTabs();
	int enterPressed();
	char* getCurrentInput();
	void end();
	void setModeClear(int, int);
	void interact(uint32_t*);
	void displayInstructions();
	int getCurrentInputLength();
	void setInputLengthLimit(int);

  private:
    Adafruit_ST7735* tft;
	Key* letters;
	Key* numbers;
	Key* specials;
	Key* bottom_keys;
	
	Key* selected;
	char* current_string; // Entered string
	char mode; // Keyboard mode
	char selected_index; // Index of selected key in list
	char current_input_length; // Length of current input
	char enter_pressed; // Indicate if enter pressed
	char last_mode; // Mode to return to when exiting tabs
	char last_key; // Key last selected when entering tabs
	char length_limit = 30;
};

#endif
