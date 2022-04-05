/*
  ST7735_Portfolio_Editor.h - Portfolio editor interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Coin.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <String.h>

// ensure this library description is only included once
#ifndef ST7735_Portfolio_Editor_h
#define ST7735_Portfolio_Editor_h

#define MAX_COINS 9

class Price_Selector {
public:
  Price_Selector(Adafruit_ST7735 *);
  void display();
  void moveLeft();
  void moveRight();
  void press();
  void setValue(double);
  double getValue();
  void increaseValueToAdd();
  void decreaseValueToAdd();
  void clear();
  void addValue();
  void subValue();
  void redrawValue();
  void redrawValueChange();
  int current_changing_index;

private:
  Adafruit_ST7735 *tft;
  int selected_index;
  int in_selector;
  double value;
  double value_to_add;
};

// library interface description
class ST7735_Portfolio_Editor {
  // user-accessible "public" interface
public:
  ST7735_Portfolio_Editor(Adafruit_ST7735 *, COIN *);
  char *press();
  int interact(uint32_t *);
  void display();
  int amount_changed;
  int checkForChange();
  void drawCoinSelected(int);
  void drawCoinUnselected(int);

  Adafruit_ST7735 *tft;
  int active;
  COIN *coins;
  int *selected_portfolio_indexes;
  int selected_portfolio_index;
  int changing_amount;
  Price_Selector *selector;
  void setActive();
};

#endif
