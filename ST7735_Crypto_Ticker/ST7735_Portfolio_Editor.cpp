/*
  ST7735_Portfolio_Editor.cpp - Portfolio editor interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Portfolio_Editor.h"

// Constructor for Portfolio Editor
ST7735_Portfolio_Editor::ST7735_Portfolio_Editor(Adafruit_ST7735 *display,
                                                 COIN *coin_arr) {
  tft = display;

  active = 0;

  coins = coin_arr;

  selector = (Price_Selector *)malloc(sizeof(Price_Selector));
  *selector = Price_Selector(tft);

  selected_portfolio_index = 0;
  changing_amount = 0;

  // Initialise portfolio arrays
  selected_portfolio_indexes = (int *)malloc(sizeof(int) * MAX_COINS+1);

  amount_changed = 0;

  for (int i = 0; i < MAX_COINS+1; i++)
    selected_portfolio_indexes[i] = -1;
}

// Sets the Portfolio Editor to be in a selected state
void ST7735_Portfolio_Editor::setActive() { active = 1; }

// Check if the amount of any coin has changed
int ST7735_Portfolio_Editor::checkForChange() {
  if (amount_changed == 1) {
    amount_changed = 0;
    return 1;
  } else {
    return 0;
  }
}

// Displays the Portfolio Editor
void ST7735_Portfolio_Editor::display() {
  tft->fillRect(0, 0, tft->width(), tft->height(), BLACK);
  tft->setTextColor(WHITE);

  int i = 0;
  while (i < COIN_COUNT) {
    tft->fillRoundRect(map(i % 3, 0, 2, 2, 2 * tft->width() / 3),
                       map(floor(i / 3), 0, 5, 5, tft->height() / 2) - 2, 3, 11,
                       2, coins[i].portfolio_colour);
    tft->setCursor(map(i % 3, 0, 2, 2, 2 * tft->width() / 3) + 7,
                   map(floor(i / 3), 0, 5, 5, tft->height() / 2));
    tft->print(coins[i].coin_code);
    i++;
  }

  drawCoinSelected(selected_portfolio_index);
}

void ST7735_Portfolio_Editor::drawCoinSelected(int index) {
  tft->setTextSize(1);
  tft->fillRoundRect(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 3,
                     map(floor(index / 3), 0, 5, 5, tft->height() / 2) - 2,
                     0.85 * (tft->width() / 3), 11, 2, DARK_GREY);
  tft->setCursor(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 7,
                 map(floor(index / 3), 0, 5, 5, tft->height() / 2));
  tft->print(coins[index].coin_code);
}

void ST7735_Portfolio_Editor::drawCoinUnselected(int index) {
  tft->setTextSize(1);
  tft->fillRoundRect(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 3,
                     map(floor(index / 3), 0, 5, 5, tft->height() / 2) - 2,
                     0.85 * (tft->width() / 3), 11, 2, BLACK);
  tft->setCursor(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 7,
                 map(floor(index / 3), 0, 5, 5, tft->height() / 2));
  tft->print(coins[index].coin_code);
}

// Interacts with the portfolio editor using the passed decoded IR data
int ST7735_Portfolio_Editor::interact(uint32_t *ir_data) {
  // Clear existing entry
  if (*ir_data == IR_ASTERISK) {
    selector->reset();
    selector->refresh();
  }

  // If # pressed when changing value do not save it and exit
  if (*ir_data == IR_HASHTAG && changing_amount == 1) {
    selector->clear();
    selector->reset();
    changing_amount = 0;
  } else if (*ir_data == IR_HASHTAG && changing_amount == 0){ // else exit
    active = 0;
    return 0;
  }

  if (changing_amount == 0) {
    // left
    if (*ir_data == IR_LEFT) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index == 0
                                     ? COIN_COUNT - 1
                                     : selected_portfolio_index - 1;
      drawCoinSelected(selected_portfolio_index);
    }

    // right
    if (*ir_data == IR_RIGHT) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index == COIN_COUNT - 1
                                     ? 0
                                     : selected_portfolio_index + 1;
      drawCoinSelected(selected_portfolio_index);
    }

    // down
    if (*ir_data == IR_DOWN) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index + 3 >= COIN_COUNT
                                     ? selected_portfolio_index
                                     : selected_portfolio_index + 3;
      drawCoinSelected(selected_portfolio_index);
    }

    // up
    if (*ir_data == IR_UP) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index - 3 < 0
                                     ? selected_portfolio_index
                                     : selected_portfolio_index - 3;
      drawCoinSelected(selected_portfolio_index);
    }

    // ok
    if (*ir_data == IR_OK) {
      selector->setOldValue(coins[selected_portfolio_index].amount);
      selector->display();
      changing_amount = 1;
    }

    drawCoinSelected(selected_portfolio_index);
  } else {
    if (selector->interact(ir_data) == 0){ // Success
      changing_amount = 0;
      coins[selected_portfolio_index].amount = selector->getValue();

      // Make sure the portfolio limit has not been exceeded
      int count = 0;
      for (int i = 0; i < COIN_COUNT; i++){
        if (coins[i].amount > 0)
          count++;
      }

      if (count > MAX_COINS)
        coins[selected_portfolio_index].amount = 0;

      selector->clear();
      selector->reset();
    }
  }
  delay(100);
  return 1;
}

// Constructor for Amount Selector
Price_Selector::Price_Selector(Adafruit_ST7735 *display) {
  tft = display;

  // Initialise new value string
  for (int i = 0; i < 13; i++){
    new_double[i] = '0';
  }
  new_double[13] = 0;
}

char Price_Selector::interact(uint32_t* ir_data){
  if (decimal_component_active == 0){
    if (input_index < 10)
      takeNumericalInput(ir_data);

    // Add decimal point and move to decimal component
    if (*ir_data == IR_OK){
      // Put a 0 before the decimal point if no inputs
      if (input_index == 0)
        input_index++;

      decimal_component_active = 1;
      decimal_point_index = input_index;
      new_double[input_index] = '.';
      input_index++;

      refresh();
    }
  } else {
    if (input_index < 13 && input_index - decimal_point_index < 7)
      takeNumericalInput(ir_data);

    // Finished input
    if (*ir_data == IR_OK) {
      return 0;
    }
  }

  Serial.println(new_double);
  return -1;
}

void Price_Selector::takeNumericalInput(uint32_t* ir_data){
  if (*ir_data == IR_ZERO){
    new_double[input_index] = '0';
    input_index++;
    refresh();
  } else if (*ir_data == IR_ONE){
    new_double[input_index] = '1';
    input_index++;
    refresh();
  } else if (*ir_data == IR_TWO){
    new_double[input_index] = '2';
    input_index++;
    refresh();
  } else if (*ir_data == IR_THREE){
    new_double[input_index] = '3';
    input_index++;
    refresh();
  } else if (*ir_data == IR_FOUR){
    new_double[input_index] = '4';
    input_index++;
    refresh();
  } else if (*ir_data == IR_FIVE){
    new_double[input_index] = '5';
    input_index++;
    refresh();
  } else if (*ir_data == IR_SIX){
    new_double[input_index] = '6';
    input_index++;
    refresh();
  } else if (*ir_data == IR_SEVEN){
    new_double[input_index] = '7';
    input_index++;
    refresh();
  } else if (*ir_data == IR_EIGHT){
    new_double[input_index] = '8';
    input_index++;
    refresh();
  } else if (*ir_data == IR_NINE){
    new_double[input_index] = '9';
    input_index++;
    refresh();
  }
}

// Sets the value to the passed value
void Price_Selector::setOldValue(double val) { 
  old_value = val; 
}

// Returns the current value
double Price_Selector::getValue() { return strtod(new_double, NULL); }

// Clears the area occupied by the selector
void Price_Selector::clear() {
  tft->fillRect(0, 92, tft->width(), tft->height()-92, BLACK);
  tft->setTextSize(2);
}

void Price_Selector::reset(){
  for (int i = 0; i < 13; i++){
    new_double[i] = '0';
  }
  new_double[13] = 0;

  decimal_component_active = 0;
  input_index = 0;
  decimal_point_index = 0;
}

// Redraws the value selected by the user
void Price_Selector::display() {
  tft->fillRect(0, 100, tft->width(), 15, BLACK);

  // Draw old value
  tft->setTextSize(1);
  tft->setTextColor(GRAY);
  tft->setCursor(2, 92);
  tft->print("Old: ");
  tft->print(old_value, 6);

  // Indicate entry
  tft->setTextColor(WHITE);
  tft->setCursor(2, 108);
  tft->setTextSize(2);
  tft->print('_');
}

void Price_Selector::refresh(){
  // Clear old value
  tft->fillRect(0, 100, tft->width(), tft->height()-100, BLACK);

  // Draw new value
  tft->setTextColor(WHITE);
  tft->setCursor(2, 108);
  tft->setTextSize(2);

  for (int i = 0; i < input_index; i++)
    tft->print(new_double[i]);

  if (input_index < 13 && (!decimal_component_active) || 
      (decimal_component_active && input_index - decimal_point_index < 7))
    tft->print('_');
}
