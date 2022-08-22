/*
  ST7735_Candle_Graph.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "Colours.h"

#define CANDLE_WIDTH 3
#define CANDLE_COUNT 26

#ifndef ESPIR_Candle_Graph_h
#define ESPIR_Candle_Graph_h

// For candle graph element
typedef struct graph_candle {
  float opening;
  float closing;
  float high;
  float low;
} G_CANDLE;

class Candle_Graph {
  public:
    Candle_Graph(Adafruit_ST7735 *, int, int, int, int);
    void display(int currency = 0);
    void displaySmall(int, int, int, int);
    G_CANDLE *candles;
    int count;
    int current_candles;
    int top;
    int bottom;
    int labels;
    void initialiseCandles();
    void nextTimePeriod(float);
    void addPrice(float);
    void freeCandles();
    int candles_init;
    void reset();

  private:
    Adafruit_ST7735 *tft;
};

#endif
