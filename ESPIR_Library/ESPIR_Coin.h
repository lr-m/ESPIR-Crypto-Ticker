/*
  ESPIR_Menu.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "ESPIR_Candle_Graph.h"
#include "ESPIR_Value_Drawer.h"
#include "Colours.h"

#ifndef ESPIR_Coin_h
#define ESPIR_Coin_h

#define COIN_COUNT 21

// Positional constants
#define BM_PRICE_START_X 60
#define NO_BM_PRICE_START_X 32
#define PRICE_START_Y 28

#define BM_CHANGE_START_X 60
#define NO_BM_CHANGE_START_X 35
#define CHANGE_START_Y 50

// For storing coins
class COIN {
public:
  COIN(char *, char *, const unsigned char *, uint16_t, uint16_t, uint16_t, 
    double, ESPIR_Value_Drawer*);
  COIN(char *, char *, uint16_t, double, ESPIR_Value_Drawer*);

  char coin_code[8];
  char coin_id[31];
  double current_price;
  double current_change;
  double amount;
  const unsigned char *bitmap;
  Candle_Graph *candles;
  uint16_t circle_colour;
  uint16_t bm_colour;
  uint16_t portfolio_colour;
  int bitmap_present;
  int bitmap_enabled;
  int candles_init;

  ESPIR_Value_Drawer* value_drawer;

  void freeCandles();
  void initCandles(Adafruit_ST7735 *);

  void display(Adafruit_ST7735 *, int currency);
  void drawPercentageChange(Adafruit_ST7735 *);
  void drawName(Adafruit_ST7735 *);
  void drawBitmap(Adafruit_ST7735 *, int16_t, int16_t, const uint8_t *, int16_t,
                  int16_t, uint16_t);
  void toggleBitmap();
  void clearIdCode();
};

#endif
