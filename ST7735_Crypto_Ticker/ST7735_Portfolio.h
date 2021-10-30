/*
  ST7735_Portfolio.h - Portfolio Interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <ST7735_Candle_Graph.h>
#include <ST7735_Coin.h>
#include <ST7735_Portfolio_Editor.h>
#include <String.h>

#define DEG2RAD 0.0174532925
#define MODE_COUNT 3

// ensure this library description is only included once
#ifndef ST7735_Portfolio_h
#define ST7735_Portfolio_h

class ST7735_Portfolio {
public:
  ST7735_Portfolio(Adafruit_ST7735 *, ST7735_Portfolio_Editor *, COIN *);
  void display();
  void getTotalValue(double *);
  float getFloatValue();
  ST7735_Portfolio_Editor *portfolio_editor;
  COIN *coins;
  int display_mode;
  void refreshSelectedCoins();
  Candle_Graph *candle_graph;
  float current_value;
  void addPriceToCandles();
  void nextTimePeriod();
  void clearCandles();
  void previousMode();
  void nextMode();

private:
  Adafruit_ST7735 *tft;
  void drawValue(double *);
  void drawBarSummary(double *);
  void drawPieSummary(double *);
  void drawCandleChart(double *);
  int fillSegment(int, int, int, int, int, unsigned int);
};

#endif
