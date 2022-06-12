/*
  ST7735_Menu.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Candle_Graph.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <String.h>

// ensure this library description is only included once
#ifndef ST7735_Coin_h
#define ST7735_Coin_h

#define COIN_COUNT 18

// Positional constants
#define PRICE_START_X 60
#define PRICE_START_Y 28

#define CHANGE_START_X 60
#define CHANGE_START_Y 50

// For storing coins
class COIN {
public:
  COIN(char *, char *, const unsigned char *, uint16_t, uint16_t, uint16_t, double);
  COIN(char *, char *, uint16_t, const unsigned char *, double);

  char coin_code[10];
  char coin_id[30];
  double current_price;
  double current_change;
  double amount;
  const unsigned char *bitmap;
  Candle_Graph *candles;
  uint16_t circle_colour;
  uint16_t bm_colour;
  uint16_t portfolio_colour;
  int bitmap_present;
  int candles_init;

  void freeCandles();
  void initCandles(Adafruit_ST7735 *);

  void display(Adafruit_ST7735 *, int currency);
  void drawPercentageChange(Adafruit_ST7735 *);
  void drawPrice(Adafruit_ST7735 *, int currency);
  void drawName(Adafruit_ST7735 *);
  void drawBitmap(Adafruit_ST7735 *, int16_t, int16_t, const uint8_t *, int16_t,
                  int16_t, uint16_t);
};

#endif
