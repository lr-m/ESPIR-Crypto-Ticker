/*
  ST7735_Portfolio_Editor.cpp - Portfolio editor interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Portfolio_Editor.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

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
  if (*ir_data == 0xF20DFF00) {
    active = 0;
    
    if (changing_amount == 1){
      // Set the value
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
      changing_amount = 0;
    }
    
    return 0;
  }

  if (changing_amount == 0) {
    // left
    if (*ir_data == 0xF708FF00) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index == 0
                                     ? COIN_COUNT - 1
                                     : selected_portfolio_index - 1;
      drawCoinSelected(selected_portfolio_index);
    }

    // right
    if (*ir_data == 0xA55AFF00) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index == COIN_COUNT - 1
                                     ? 0
                                     : selected_portfolio_index + 1;
      drawCoinSelected(selected_portfolio_index);
    }

    // down
    if (*ir_data == 0xAD52FF00) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index + 3 >= COIN_COUNT
                                     ? selected_portfolio_index
                                     : selected_portfolio_index + 3;
      drawCoinSelected(selected_portfolio_index);
    }

    // up
    if (*ir_data == 0xE718FF00) {
      drawCoinUnselected(selected_portfolio_index);
      selected_portfolio_index = selected_portfolio_index - 3 < 0
                                     ? selected_portfolio_index
                                     : selected_portfolio_index - 3;
      drawCoinSelected(selected_portfolio_index);
    }

    // ok
    if (*ir_data == 0xE31CFF00) {
      selector->setValue(coins[selected_portfolio_index].amount);
      selector->display();
      changing_amount = 1;
    }

    drawCoinSelected(selected_portfolio_index);
  } else {
    // left
    if (*ir_data == 0xF708FF00)
      selector->increaseValueToAdd();

    // right
    if (*ir_data == 0xA55AFF00)
      selector->decreaseValueToAdd();

    // up
    if (*ir_data == 0xE718FF00) {
      selector->addValue();
      amount_changed = 1;
    }

    // down
    if (*ir_data == 0xAD52FF00) {
      selector->subValue();
      amount_changed = 1;
    }

    // ok
    if (*ir_data == 0xE31CFF00) {
      changing_amount = 0;

      // Set the value
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
    }
  }
  delay(100);
  return 1;
}

// Constructor for Amount Selector
Price_Selector::Price_Selector(Adafruit_ST7735 *display) {
  tft = display;

  selected_index = 0;
  value = 0;
  value_to_add = 1;
}

// Adds the currently selected value
void Price_Selector::addValue() {
  value = value + value_to_add < 99999999999 ? value + value_to_add : value;
  redrawValue();
}

// Subtracts the currently selected value
void Price_Selector::subValue() {
  value = value - value_to_add >= 0 ? value - value_to_add : 0;
  redrawValue();
}

// Display the selector on the screen
void Price_Selector::display() {
  redrawValue();
  redrawValueChange();
}

// Sets the value to the passed value
void Price_Selector::setValue(double val) { value = val; }

// Returns the current value
double Price_Selector::getValue() { return value; }

// Multiplies the selected value to add by 10
void Price_Selector::increaseValueToAdd() {
  value_to_add =
      value_to_add * 10 < 10000000000 ? value_to_add * 10 : value_to_add;
  redrawValueChange();
}

// Divides the selected value to add by 10
void Price_Selector::decreaseValueToAdd() {
  value_to_add =
      value_to_add / 10 >= 0.000001 ? value_to_add / 10 : value_to_add;
  redrawValueChange();
}

// Clears the area occupied by the selector
void Price_Selector::clear() {
  tft->fillRect(0, 98, tft->width(), 40, BLACK);
  tft->setTextSize(2);
}

// Redraws the value selected by the user
void Price_Selector::redrawValue() {
  tft->fillRect(0, 98, tft->width(), 15, BLACK);
  tft->setTextColor(WHITE);
  tft->setCursor(2, 98);
  tft->setTextSize(2);

  if (value >= 1000000) {
    tft->print(value, 0);
  } else {
    tft->print(value, 6);
  }
}

// Redraws the value change selected by the user
void Price_Selector::redrawValueChange() {
  tft->setTextSize(1);
  tft->fillRect(0, 116, tft->width(), 8, BLACK);
  tft->setCursor(2, 116);
  tft->setTextColor(GREEN);
  tft->print('+');
  tft->setTextColor(RED);
  tft->print('-');
  tft->setTextColor(WHITE);
  tft->print(value_to_add, 6);
}
