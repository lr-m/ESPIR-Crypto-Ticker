/*
  ST7755_Menu.h - Keyboard library for ST7755 display
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7755_Menu.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

//////////////////////////////////////////////// Menu ///////////////////////////////////////////////

// Constructor for Keyboard
ST7755_Menu::ST7755_Menu(Adafruit_ST7735* display, int btn_count, char** button_values)
{
	tft = display;
	
	button_count = btn_count;
	values = button_values;
	
	buttons = (Button*) malloc(sizeof(Button)*button_count);

	for (int i = 0; i < button_count; i++){
		buttons[i] = Button(tft, 0, 4+i*22, tft->width(), 20, button_values[i], 1);
	}
	
	selected_button_index = 0;
}

// Performs the action of the currently selected key
char* ST7755_Menu::press(){
	return buttons[selected_button_index].action;
}

// Display the menu on the screen
void ST7755_Menu::display()
{
	tft->fillRect(0, 0, tft->width(), tft->height()-20, KEYBOARD_BG_COLOUR);
	tft->setTextColor(ST77XX_WHITE);
	tft->setTextSize(1);
	for (int i = 0; i < button_count; i++){
		if (i == selected_button_index){
			buttons[i].displaySelected();
		} else {
			buttons[i].display();
		}
	}
}

// Move down to the next button
void ST7755_Menu::moveDown(){
	buttons[selected_button_index].display();
	selected_button_index = (selected_button_index + 1)%button_count;
	buttons[selected_button_index].displaySelected();
}

// Move up to the above button
void ST7755_Menu::moveUp(){
	buttons[selected_button_index].display();
	selected_button_index = selected_button_index == 0 ? button_count-1 : selected_button_index - 1;
	buttons[selected_button_index].displaySelected();
}

// Get the list of buttons
Button* ST7755_Menu::getButtons(){
	return buttons;
}

////////////////////////////////////////////// Button ///////////////////////////////////////////

// Constructor for Button
Button::Button(Adafruit_ST7735* display, int x_pos, int y_pos, int width, 
		int height, char* act, int return_sub_button){
	tft = display;
	x = x_pos;
	y = y_pos;
	w = width;
	h = height;
	action = act;
	selector_count = 0;
	selectors = (Selector*) malloc(sizeof(Selector)*MAX_SELECTORS);
	if (return_sub_button == 1){
		return_button = (Button*) malloc(sizeof(Button));
		*return_button = Button(tft, 0, 0, tft->width(), 20, "Return", 0);
	}
	on_return_button = 1;
	current_selector = 1;
}

// Display the Button
void Button::display(){
	tft->fillRoundRect(x, y, w, h, 2, UNSELECTED_BG_COLOUR);
	tft->setCursor(x+8, y+6);
	tft->setTextColor(UNSELECTED_LETTER_COLOUR);
	tft->print(action);
}

// Display the Button as selected
void Button::displaySelected(){
	tft->fillRoundRect(x, y, w, h, 2, SELECTED_BG_COLOUR);
	tft->setCursor(x+8, y+6);
	tft->setTextColor(SELECTED_LETTER_COLOUR);
	tft->print(action);
}

// Add a selector to the submenu of the button
void Button::addSelector(char* prompt, char** options, 
		int window_size, int max, int vals){
	if (selector_count < MAX_SELECTORS){
		selector_count++;
		selectors[selector_count-1] = Selector(tft, 0, 
			(selector_count)*40, prompt, options, window_size, max, vals);
	}
}

// Draw the buttons sub menu
void Button::drawSubMenu(){
	tft->fillRect(0, 0, tft->width(), tft->height()-20, KEYBOARD_BG_COLOUR);
	return_button->display();
	
	for (int i = 0; i < selector_count; i++){
		if (i == current_selector && on_return_button == 0){
			selectors[i].displaySelected();
		} else {
			selectors[i].display();
		}
	}
}

// Press the currently selected button on the sub menu
char* Button::pressSubMenu(){
	if (on_return_button == 1){
		return return_button->action;
	} else {
		selectors[current_selector].press();
	}
}

// Flash the current selected option
void Button::flashSelectedSelector(){
	if (on_return_button == 0){
		selectors[current_selector].flashSelected();
	}
}

// Move down to the next component in the sub menu
void Button::subMenuDown(){
	if (on_return_button == 1){
		on_return_button = 0;
		current_selector = 0;
		return_button->display();
	} else if (current_selector < selector_count-1){
		selectors[current_selector].display();
		current_selector++;
	}
	
	selectors[current_selector].displaySelected();
}

// Move up to the above component in the sub menu
void Button::subMenuUp(){
	if (on_return_button == 0){
		if (current_selector == 0){
			selectors[current_selector].display();
			on_return_button = 1;
			return_button->displaySelected();
		} else {
			selectors[current_selector].display();
			current_selector--;
			selectors[current_selector].displaySelected();
		}
	}
}

// Selected the option on the left (if possible) in sub menu
void Button::subMenuLeft(){
	if (on_return_button == 0){
		selectors[current_selector].moveLeft();
	}
}

// Selected the option on the right (if possible) in sub menu
void Button::subMenuRight(){
	if (on_return_button == 0){
		selectors[current_selector].moveRight();
	}
}

////////////////////////////////////////////// Selector ///////////////////////////////////////////

Selector::Selector(Adafruit_ST7735* display, int x_pos, int y_pos, 
		char* act, char** opt, int size, int max, int count){
	tft = display;
	x = x_pos;
	y = y_pos;
	prompt = act;
	options = opt;
	selected_index = 0;
	window_size = size;
	first_index = 0;
	max_selected = max;
	current_changing_index = 0;
	value_count = count;
	
	// Selected index init
	selected_indexes = (int*) malloc(sizeof(int) * max);
	selected_indexes[0] = 0; // First selected by default
	for (int i = 1; i < max; i++){
		selected_indexes[i] = -1;
	}
}

// Redraw the buttons of the selector
void Selector::cycleButtons(){
	for (int i = first_index; i <= (first_index + window_size)-1; i++){
		tft->setCursor(map(i, first_index, first_index + window_size-1, FIRST_BUTTON_X, 
			tft->width()-LAST_BUTTON_X)-3, y+13);
		
		int selected_test = 0;
		for (int j = 0; j < max_selected; j++){
			if (selected_indexes[j] == i){
				selected_test = 1;
				break;
			}
		}
		
		if (selected_test == 0){ // Not selected so display red background
			tft->fillRoundRect(map(i, first_index, first_index + window_size-1, FIRST_BUTTON_X, 
				tft->width()-LAST_BUTTON_X)-2, y+14, (tft->width()-LAST_BUTTON_X)/window_size, 12, 2, RED);
		} else { // Selected so display green background
			tft->fillRoundRect(map(i, first_index, first_index + window_size-1, FIRST_BUTTON_X, 
				tft->width()-LAST_BUTTON_X)-2, y+14, (tft->width()-LAST_BUTTON_X)/window_size, 12, 2, GREEN);
		}
		
		tft->setCursor(map(i, first_index, first_index + window_size-1, FIRST_BUTTON_X, 
			tft->width()-LAST_BUTTON_X), y+16);
		tft->print(options[i]);
	}
}

// Display the selector
void Selector::display(){
	tft->setCursor(x+5, y);
	tft->fillRect(x, y-3, tft->width(), 35, ST77XX_BLACK);
	tft->print(prompt);
	
	this->cycleButtons();
}

// Display the selector as selected
void Selector::displaySelected(){
	tft->setCursor(x+5, y);
	tft->fillRect(x, y-3, tft->width(), 35, DARK_GREY);
	tft->print(prompt);
	
	this->cycleButtons();
}

// Flash the selected option
void Selector::flashSelected(){
	tft->setCursor(map(current_changing_index, first_index, 
		first_index + window_size-1, FIRST_BUTTON_X, tft->width()-LAST_BUTTON_X)-3, y+13);
	
	int selected_test = 0;
	for (int j = 0; j < max_selected; j++){
		if (selected_indexes[j] == current_changing_index){
			selected_test = 1;
			break;
		}
	}
	
	tft->fillRoundRect(map(current_changing_index, first_index, 
		first_index + window_size-1, FIRST_BUTTON_X, tft->width()-LAST_BUTTON_X)-2, y+14, 
		(tft->width()-LAST_BUTTON_X)/window_size, 12, 2, DARK_GREY);
	tft->setCursor(map(current_changing_index, first_index, 
		first_index + window_size-1, FIRST_BUTTON_X, tft->width()-LAST_BUTTON_X), y+16);
	tft->print(options[current_changing_index]);
	delay(150);
	if (selected_test == 0){ // Not selected so display red background
		tft->fillRoundRect(map(current_changing_index, first_index, 
			first_index + window_size-1, FIRST_BUTTON_X, tft->width()-LAST_BUTTON_X)-2, y+14, 
			(tft->width()-LAST_BUTTON_X)/window_size, 12, 2, RED);
	} else { // Selected so display green background
		tft->fillRoundRect(map(current_changing_index, first_index, 
			first_index + window_size-1, FIRST_BUTTON_X, tft->width()-LAST_BUTTON_X)-2, y+14, 
			(tft->width()-LAST_BUTTON_X)/window_size, 12, 2, GREEN);
	}
	
	
	tft->setCursor(map(current_changing_index, first_index, 
		first_index + window_size-1, FIRST_BUTTON_X, tft->width()-LAST_BUTTON_X), y+16);
	tft->print(options[current_changing_index]);
	
	delay(250);
}

// Move to the option on the left
void Selector::moveLeft(){
	if (current_changing_index > 0){
		current_changing_index--;
	}
	
	if (current_changing_index == first_index - 1 && first_index > 0){
		first_index--;
		this->cycleButtons();
	}
}

// Move to the option on the right
void Selector::moveRight(){
	if (current_changing_index < value_count-1){
		current_changing_index++;
	}
	
	if (current_changing_index == first_index + window_size && 
			first_index + window_size < value_count){
		first_index++;
		this->cycleButtons();
	}
}

// Press the current selected button
void Selector::press(){
	Serial.println("pressed");
	if (max_selected == 1){
		selected_indexes[0] = current_changing_index;
	} else {
		for (int i = 0; i < max_selected; i++){
			if (selected_indexes[i] == current_changing_index){
				if (i == 1 && selected_indexes[1] == -1){ // Make sure at least 1 thing pressed
					return;
				}
				
				for (int j = i; j < max_selected-1; j++){
					selected_indexes[j] = selected_indexes[j+1];
				}
				selected_indexes[max_selected-1] = -1;
				
				for (int i = 0; i < max_selected; i++){
					Serial.print(selected_indexes[i]);
					Serial.print(" ");
				}
				
				return;
			}
		}
		
		for (int i = max_selected-1; i > 0; i--){
			selected_indexes[i] = selected_indexes[i-1];
		}
		selected_indexes[0] = current_changing_index;
	}
	
	for (int i = 0; i < max_selected; i++){
		Serial.print(selected_indexes[i]);
		Serial.print(" ");
	}
	Serial.println();
	this->cycleButtons();
}

// Return the indexes of the selected elements
int* Selector::getSelected(){
	return selected_indexes;
}