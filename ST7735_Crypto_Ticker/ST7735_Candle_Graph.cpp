/*
  ST7735_Candle_Graph.h - Candle graph for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Candle_Graph.h"
#include "HardwareSerial.h"
#include "ST7735_Portfolio_Editor.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

// Constructor for Portfolio Editor
Candle_Graph::Candle_Graph(Adafruit_ST7735 *display, int candle_count,
                           int top_y, int bot_y, int labels_val) {
  tft = display;

  if (labels == 1) {
    top = top_y + 12;
    bottom = bot_y - 14;
  } else {
    top = top_y;
    bottom = bot_y;
  }

  current_candles = 1;

  labels = labels_val;

  candles_init = 0;

  count = candle_count;

  candles = (G_CANDLE *)malloc(sizeof(G_CANDLE) * candle_count);

  initialiseCandles();
}

// Clear all data from the candles
void Candle_Graph::reset() {
  initialiseCandles();
  current_candles = 1;
}

// Initialise candles by setting -1 to all values
void Candle_Graph::initialiseCandles() {
  candles_init = 1;
  for (int i = 0; i < count; i++) {
    candles[i].opening = -1;
    candles[i].closing = -1;
    candles[i].low = -1;
    candles[i].high = -1;
  }
}

// Free the candles
void Candle_Graph::freeCandles() {
  if (candles_init == 1) {
    candles_init = 0;
    free(candles);
  }
}

// Display line chart version
void Candle_Graph::displaySmall(int x, int w, int top, int bottom, int upflag){
  double max_val = -1;
  double min_val = std::numeric_limits<double>::max();

  if (current_candles <= 1){
    return;
  }

  for (int i = count - current_candles; i < count; i++) {
    if (candles[i].high > max_val)
      max_val = candles[i].high;

    if (candles[i].low < min_val && candles[i].low >= 0)
      min_val = candles[i].low;
  }

  int last_y = ((candles[count - current_candles].high + candles[count - current_candles].low)/2 - min_val) * 
    ((double)top - (double)bottom) / (max_val - min_val) + (double)bottom;

  if (max_val - min_val == 0) last_y = (top+bottom)/2;

  for (int i = count - current_candles + 1; i < count; i++) {
    int point_y;

    if (min_val != max_val) {
      point_y = ((candles[i].high + candles[i].low)/2 - min_val) * ((double)top - (double)bottom) /
        (max_val - min_val) + (double)bottom;
    } else {
      point_y = (top + bottom) / 2;
    }
    
    if (candles[i].low < 0) continue;

    if (point_y <= last_y){
      tft->drawLine(map(i-1, 0, count, x, x + w), last_y, map(i, 0, count, x, x + w), point_y, ST77XX_GREEN);
    } else {
      tft->drawLine(map(i-1, 0, count, x, x + w), last_y, map(i, 0, count, x, x + w), point_y, RED);
    }

    last_y = point_y;
  }
}

// Sets the Portfolio Editor to be in a selected state (full width of screen)
void Candle_Graph::display(int currency) {
  double max_val = -1;
  double min_val = std::numeric_limits<double>::max();

  for (int i = count - current_candles; i < count; i++) {
    if (candles[i].high > max_val)
      max_val = candles[i].high;

    if (candles[i].low < min_val && candles[i].low >= 0)
      min_val = candles[i].low;
  }

  if (labels == 1) {
    tft->setTextSize(1);
    tft->setTextColor(ST77XX_GREEN);
    tft->setCursor(0, top - 10);
    
    if (currency == 0){ // GBP
      tft->print(char(156));
    } else if (currency == 1){ // USD
      tft->print(char(36));
    } else if (currency == 2){ // EUR
      tft->print(char(237));
    }

    tft->print(max_val);
    tft->setTextColor(RED);
    tft->setCursor(0, bottom + 2);
    
    if (currency == 0){ // GBP
      tft->print(char(156));
    } else if (currency == 1){ // USD
      tft->print(char(36));
    } else if (currency == 2){ // EUR
      tft->print(char(237));
    }

    tft->print(min_val);
    tft->setTextColor(WHITE);
  }

  for (int i = count - current_candles; i < count; i++) {
    int opening_y;
    int closing_y;
    int high_y;
    int low_y;

    if (min_val != max_val) {
      opening_y = (candles[i].opening - min_val) *
                      ((double)top - (double)bottom) / (max_val - min_val) +
                  (double)bottom;
      closing_y = (candles[i].closing - min_val) *
                      ((double)top - (double)bottom) / (max_val - min_val) +
                  (double)bottom;
      high_y = (candles[i].high - min_val) * ((double)top - (double)bottom) /
                   (max_val - min_val) +
               (double)bottom;
      low_y = (candles[i].low - min_val) * ((double)top - (double)bottom) /
                  (max_val - min_val) +
              (double)bottom;
    } else {
      opening_y = (top + bottom) / 2;
      closing_y = (top + bottom) / 2;
      high_y = (top + bottom) / 2;
      low_y = (top + bottom) / 2;
    }

    if (candles[i].low < 0) continue;

    int bar_x = map(i, 0, count, 2, tft->width() - 2);
    uint16_t color;

    color = (candles[i].opening > candles[i].closing) ? RED : ST77XX_GREEN;

    tft->drawLine(bar_x, high_y, bar_x + 2, high_y, color);
    tft->drawLine(bar_x + 1, high_y, bar_x + 1, min(opening_y, closing_y),
                  color);
    tft->fillRect(bar_x, min(opening_y, closing_y), CANDLE_WIDTH,
                  max(opening_y, closing_y) - min(opening_y, closing_y), color);
    tft->drawLine(bar_x + 1, low_y, bar_x + 1, max(opening_y, closing_y),
                  color);
    tft->drawLine(bar_x, low_y, bar_x + 2, low_y, color);
  }
}

// Add a price to the candle graph
void Candle_Graph::addPrice(float val) {
  if (candles[count - 1].opening == -1) {
    candles[count - 1].opening = val;
    candles[count - 1].closing = val;
    candles[count - 1].high = val;
    candles[count - 1].low = val;
  }

  candles[count - 1].closing = val;

  if (val > candles[count - 1].high)
    candles[count - 1].high = val;

  if (val < candles[count - 1].low)
    candles[count - 1].low = val;
}

// Move to the next time period
void Candle_Graph::nextTimePeriod(float current_price) {
  for (int i = 1; i < count; i++) {
    candles[i - 1].opening = candles[i].opening;
    candles[i - 1].closing = candles[i].closing;
    candles[i - 1].high = candles[i].high;
    candles[i - 1].low = candles[i].low;
  }

  candles[count - 1].opening = current_price;
  candles[count - 1].high = current_price;
  candles[count - 1].low = current_price;
  candles[count - 1].closing = current_price;

  current_candles = min(count, current_candles + 1);
}
