/*
  ESPIR_Portfolio.h - Portfolio Interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <cmath>

#include "ESPIR_Candle_Graph.h"
#include "ESPIR_Coin.h"
#include "ESPIR_Portfolio_Editor.h"
#include "Colours.h"

#define DEG2RAD 0.0174532925
#define MODE_COUNT 3

// ensure this library description is only included once
#ifndef ESPIR_Portfolio_h
#define ESPIR_Portfolio_h

class ESPIR_Portfolio {
public:
  ESPIR_Portfolio(Adafruit_ST7735 *, ESPIR_Portfolio_Editor *, COIN *, ESPIR_Value_Drawer*);
  void display(int);
  void getTotalValue(double *);
  float getFloatValue();
  ESPIR_Portfolio_Editor *portfolio_editor;
  COIN *coins;
  int display_mode;
  void refreshSelectedCoins();
  Candle_Graph *candle_graph;
  void addPriceToCandles();
  void nextTimePeriod();
  void clearCandles();
  void previousMode();
  void nextMode();
  void drawPropBar(double *);

  ESPIR_Value_Drawer* value_drawer;

private:
  Adafruit_ST7735 *tft;
  void drawValue(double *, int);
  void drawBarSummary(double *, int);
  void drawPieSummary(double *);
  void drawCandleChart(double *, int);
  int fillSegment(int, int, int, int, int, unsigned int);
};

#endif
