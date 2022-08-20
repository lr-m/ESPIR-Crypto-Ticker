/*
  ST7735_Portfolio_Editor.h - Portfolio editor interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "ST7735_Coin.h"
#include "Colours.h"
#include "IR_Codes.h"

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
  void setOldValue(double);
  double getValue();
  void clear();
  void takeNumericalInput(uint32_t*);
  void reset();
  char interact(uint32_t*);
  void refresh();

private:
  Adafruit_ST7735 *tft;
  double old_value;

  char new_double[14];
  char decimal_component_active = 0;
  char input_index = 0;
  char decimal_point_index = 0;
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
