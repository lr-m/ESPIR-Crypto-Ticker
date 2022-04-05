/*
  ST7735_Portfolio.cpp - Portfolio interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Portfolio.h"
#include "HardwareSerial.h"
#include "ST7735_Candle_Graph.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>
#include <cmath>

// Constructor for Portfolio Editor
ST7735_Portfolio::ST7735_Portfolio(Adafruit_ST7735 *display,
                                   ST7735_Portfolio_Editor *editor,
                                   COIN *coin_arr) {
  tft = display;
  portfolio_editor = editor;
  coins = coin_arr;

  candle_graph = (Candle_Graph *)malloc(sizeof(Candle_Graph));
  *candle_graph = Candle_Graph(tft, CANDLE_COUNT, 36, tft->height() - 10, 1);

  display_mode = 0;

  current_value = 0;
}

// Draw the current value of the portfolio
void ST7735_Portfolio::drawValue(double *total_value) {
  tft->setTextSize(2);
  tft->setTextColor(WHITE);
  tft->setCursor(5, 4);

  tft -> print(char(156));
  if (*total_value >= 100000000) {
    tft -> print(String(*total_value, 0));
  } else {
    tft -> print(*total_value);
  }
}

// Moves to the next display mode
void ST7735_Portfolio::nextMode() {
  display_mode++;
  if (display_mode > MODE_COUNT - 1)
    display_mode = 0;
}

// Moves to the previous display mode
void ST7735_Portfolio::previousMode() {
  display_mode--;
  if (display_mode < 0)
    display_mode = MODE_COUNT - 1;
}

// Draws the candle chart
void ST7735_Portfolio::drawCandleChart(double *total_value) {
  // drawPropBar(total_value);

  candle_graph->display();
}

void ST7735_Portfolio::drawPropBar(double *total_value) {
  double coin_total = 0;
  double current_total = 0;
  int i = 0;

  //Draw bar proportional to amount of coin owned
  while (portfolio_editor->selected_portfolio_indexes[i] != -1) {
    coin_total =
        coins[portfolio_editor->selected_portfolio_indexes[i]].current_price *
        coins[portfolio_editor->selected_portfolio_indexes[i]].amount;

    tft->fillRect(map(current_total, 0, *total_value, 0, tft->width()), 24,
                  1 + map(coin_total, 0, *total_value, 0, tft->width()), 3,
                  coins[portfolio_editor->selected_portfolio_indexes[i]]
                      .portfolio_colour);

    current_total += coin_total;
    i++;
  }
}

// Moves the candles to the next time period
void ST7735_Portfolio::nextTimePeriod() {
  candle_graph->nextTimePeriod(getFloatValue());
}

// Adds the current price of portfolio to the candles
void ST7735_Portfolio::addPriceToCandles() {
  float val = getFloatValue();

  candle_graph->addPrice(val);
}

// Clears the candles
void ST7735_Portfolio::clearCandles() { candle_graph->reset(); }

// Display the coin breakdown screen with bar proportional bar chart
void ST7735_Portfolio::drawBarSummary(double *total_value) {
  drawPropBar(total_value);

  // Draw coin code, cost of owned, and % of portfolio
  int i = 0;
  double coin_total = 0;

  while (portfolio_editor->selected_portfolio_indexes[i] != -1) {
    coin_total =
        coins[portfolio_editor->selected_portfolio_indexes[i]].current_price *
        coins[portfolio_editor->selected_portfolio_indexes[i]].amount;
    tft->setTextSize(1);
    tft->setCursor(7, 33 + i * 10);
    tft->fillRect(2, 32 + i * 10, 2, 10,
                  coins[portfolio_editor->selected_portfolio_indexes[i]]
                      .portfolio_colour);
    tft -> print(
        coins[portfolio_editor->selected_portfolio_indexes[i]].coin_code);
    tft->setCursor(52, 33 + i * 10);
    tft->setTextColor(WHITE);
    tft -> print(char(156));

    if (coin_total >= 100000000) {
      tft -> print(String(coin_total, 0));
    } else if (coin_total >= 10000000) {
      tft -> print(String(coin_total, 1));
    } else {
      tft -> print(String(coin_total, 2));
    }

    tft -> setCursor(123, 33 + i * 10);

    if (coins[portfolio_editor->selected_portfolio_indexes[i]].current_change <
        0) {
      tft->setTextColor(RED);
    } else {
      tft->setTextColor(ST77XX_GREEN);
    }

    if (coin_total < 10000000000) {
      if (coins[portfolio_editor->selected_portfolio_indexes[i]]
              .current_change > 0) {
        tft -> print('+');
      }

      if (abs(coins[portfolio_editor->selected_portfolio_indexes[i]]
                  .current_change) >= 1000) {
        tft -> print(String(coins[portfolio_editor->selected_portfolio_indexes[i]]
                              .current_change,
                          0));
      } else if (abs(coins[portfolio_editor->selected_portfolio_indexes[i]]
                         .current_change) >= 100) {
        tft -> print(String(coins[portfolio_editor->selected_portfolio_indexes[i]]
                              .current_change,
                          0));
      } else if (abs(coins[portfolio_editor->selected_portfolio_indexes[i]]
                         .current_change) >= 10) {
        tft -> print(String(coins[portfolio_editor->selected_portfolio_indexes[i]]
                              .current_change,
                          1));
      } else if (abs(coins[portfolio_editor->selected_portfolio_indexes[i]]
                         .current_change) >= 0) {
        tft -> print(String(coins[portfolio_editor->selected_portfolio_indexes[i]]
                              .current_change,
                          2));
      }

      tft -> print('%');
      tft->setTextColor(WHITE);
    }
    i++;
  }
}

// Display the proportion screen with pie chart
void ST7735_Portfolio::drawPieSummary(double *total_value) {
  double coin_total = 0;
  double current_total = 0;
  int i = 0;

  // Draw coin code, cost of owned, and % of portfolio
  i = 0;
  while (portfolio_editor->selected_portfolio_indexes[i] != -1) {
    coin_total =
        coins[portfolio_editor->selected_portfolio_indexes[i]].current_price *
        coins[portfolio_editor->selected_portfolio_indexes[i]].amount;
    tft -> setTextSize(1);
    tft -> setCursor(7, 30 + i * 10);
    tft -> fillRect(2, 29 + i * 10, 2, 10,
                  coins[portfolio_editor->selected_portfolio_indexes[i]]
                      .portfolio_colour);
    tft -> print(
        coins[portfolio_editor->selected_portfolio_indexes[i]].coin_code);
    tft -> setCursor(50, 30 + i * 10);
    tft -> setTextColor(WHITE);
    tft -> print(String(coin_total / (*total_value) * 100, 1));
    tft -> print('%');
    i++;
  }

  // Draw pi chart proportional to amount of coin owned
  i = 0;
  while (portfolio_editor->selected_portfolio_indexes[i] != -1) {
    coin_total =
        coins[portfolio_editor->selected_portfolio_indexes[i]].current_price *
        coins[portfolio_editor->selected_portfolio_indexes[i]].amount;

    fillSegment(tft->width() / 2 + 40, tft->height() / 2 + 12,
                map(current_total, 0, *total_value, 0, 360),
                map(coin_total, 0, *total_value, 0, 360), 35,
                coins[portfolio_editor->selected_portfolio_indexes[i]]
                    .portfolio_colour);

    current_total += coin_total;
    i++;
  }
}

// Display the portfolio
void ST7735_Portfolio::display() {
  tft->fillRect(0, 0, tft->width(), tft->height(), BLACK);

  // If in portfolio mode, draw the portfolio upon update
  double total_value = 0;
  getTotalValue(&total_value);
  drawValue(&total_value);

  // Check that there are coins with non-zero values owned
  if (portfolio_editor->selected_portfolio_indexes[0] != -1) {
    int i = 0;
    double coin_price_owned[MAX_COINS];

    for (int i = 0; i < MAX_COINS; i++){
      coin_price_owned[i] = 0;
    }

    // Store cost of each owned coin
    while (portfolio_editor->selected_portfolio_indexes[i] != -1) {
      coin_price_owned[i] =
          coins[portfolio_editor->selected_portfolio_indexes[i]].current_price *
          coins[portfolio_editor->selected_portfolio_indexes[i]].amount;
      i++;
    }

    // Sort by price of owned coin (simple bubble sort)
    double temp = 0;
    for (int k = 0; k < i - 1; k++) {
      for (int j = 0; j < i - k - 1; j++) {
        if (coin_price_owned[j + 1] > coin_price_owned[j]) {
          temp = portfolio_editor->selected_portfolio_indexes[j];
          portfolio_editor->selected_portfolio_indexes[j] =
              portfolio_editor->selected_portfolio_indexes[j + 1];
          portfolio_editor->selected_portfolio_indexes[j + 1] = temp;

          temp = coin_price_owned[j];
          coin_price_owned[j] = coin_price_owned[j + 1];
          coin_price_owned[j + 1] = temp;
        }
      }
    }

    if (display_mode == 0) {
      drawBarSummary(&total_value);
    } else if (display_mode == 1) {
      drawPieSummary(&total_value);
    } else if (display_mode == 2) {
      drawCandleChart(&total_value);
    }
  }
}

// Refreshes the selected coins, adding any new coins that may have been added
void ST7735_Portfolio::refreshSelectedCoins() {
  // Clear selected coins
  for (int i = 0; i < MAX_COINS; i++)
    portfolio_editor->selected_portfolio_indexes[i] = -1;

  // Add up to 9 non-zero coins to selected in portfolio
  int curr_selected_port_index = 0;
  for (int i = 0; i < COIN_COUNT; i++) {
    if (coins[i].amount > 0) {
      portfolio_editor->selected_portfolio_indexes[curr_selected_port_index] =
          i;
      curr_selected_port_index++;

      if (curr_selected_port_index == MAX_COINS+1)
        break;
    }
  }
}

// Store the current portfolio as a double in the passed double
void ST7735_Portfolio::getTotalValue(double *total_value) {
  int i = 0;
  while (portfolio_editor->selected_portfolio_indexes[i] != -1) {
    *total_value +=
        coins[portfolio_editor->selected_portfolio_indexes[i]].current_price *
        coins[portfolio_editor->selected_portfolio_indexes[i]].amount;
    i++;
  }
}

// Get the current total portfolio value as a float
float ST7735_Portfolio::getFloatValue() {
  int i = 0;
  float value = 0;
  while (portfolio_editor->selected_portfolio_indexes[i] != -1) {
    value +=
        coins[portfolio_editor->selected_portfolio_indexes[i]].current_price *
        coins[portfolio_editor->selected_portfolio_indexes[i]].amount;
    i++;
  }
  return value;
}

// Draws a segment (for the pie chart)
int ST7735_Portfolio::fillSegment(int x, int y, int startAngle, int subAngle,
                                  int r, unsigned int colour) {
  float sx = cos((startAngle - 90) * DEG2RAD);
  float sy = sin((startAngle - 90) * DEG2RAD);
  uint16_t x1 = sx * r + x;
  uint16_t y1 = sy * r + y;

  for (int i = startAngle; i < startAngle + subAngle; i++) {
    int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
    int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;
    tft->fillTriangle(x1, y1, x2, y2, x, y, colour);
    x1 = x2;
    y1 = y2;
  }
}
