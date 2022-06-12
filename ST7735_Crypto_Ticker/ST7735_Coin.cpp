#include "ST7735_Coin.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

// Constructor for Coin with bitmap
COIN::COIN(char* code, char *id, const unsigned char* bm, uint16_t circle_col,
           uint16_t bm_col, uint16_t portfolio_col, double coin_amount) {

  int i = 0;
  while(code[i] != 0){
    coin_code[i] = code[i];
    i++;
  }
  coin_code[i] = 0;

  i = 0;
  while(id[i] != 0){
    coin_id[i] = id[i];
    i++;
  }
  coin_id[i] = 0;

  bitmap = bm;
  circle_colour = circle_col;
  bm_colour = bm_col;
  portfolio_colour = portfolio_col;
  amount = coin_amount;
  candles_init = 0;

  bitmap_present = 1;
}

// Constructor for coin without bitmap
COIN::COIN(char *code, char *id, uint16_t portfolio_col,
           const unsigned char *bm, double coin_amount) {
  int i = 0;
  while(code[i] != 0){
    coin_code[i] = code[i];
    i++;
  }
  coin_code[i] = 0;

  i = 0;
  while(id[i] != 0){
    coin_id[i] = id[i];
    i++;
  }
  coin_id[i] = 0;

  portfolio_colour = portfolio_col;
  amount = coin_amount;
  bitmap = bm;
  bitmap_present = 0;
}

// Free the candles
void COIN::freeCandles() {
  if (candles_init == 1) {
    candles_init = 0;
    candles->freeCandles();
    free(candles);
  }
}

// Initialise the candle graph
void COIN::initCandles(Adafruit_ST7735 *display) {
  if (candles_init == 0) {
    candles_init = 1;
    candles = (Candle_Graph *)malloc(sizeof(Candle_Graph));
    *(candles) = Candle_Graph(display, 26, display->height() / 2,
                              display->height() - 10, 0);
  }
}

// Display properties of coin on display
void COIN::display(Adafruit_ST7735 *display, int currency) {
  display->setTextColor(WHITE);
  display->fillRect(0, 0, display->width(), display->height(), BLACK);

  if (bitmap_present == 1) {
    display->fillCircle(12, 12, 44, circle_colour);
    drawBitmap(display, 4, 4, bitmap, 40, 40, bm_colour);
  } else {
    drawBitmap(display, 0, 0, bitmap, 56, 56, portfolio_colour);
  }

  drawName(display);
  drawPrice(display, currency);
  drawPercentageChange(display);
  candles->display();
}

// Draws the name of the coin on the screen.
void COIN::drawName(Adafruit_ST7735 *display) {
  display->setTextSize(2);

  int len = 0;
  for (len = 0; len < 8; len++) {
    if (coin_code[len] == 0)
      break;
  }

  if (len & 2 == 0) {
    display->setCursor(
        PRICE_START_X +
            (((display->width() - PRICE_START_X) / 8) * (8 - len)) / 2,
        6);
  } else {
    display->setCursor(
        PRICE_START_X +
            (((display->width() - PRICE_START_X) / 8) * (8 - len)) / 2,
        6);
  }

  display->print(coin_code);
}

// Draws the percentage change on the screen.
void COIN::drawPercentageChange(Adafruit_ST7735 *display) {
  display->setCursor(CHANGE_START_X, CHANGE_START_Y);
  display->setTextSize(1);
  display->print("24 Hour: ");
  if (current_change < 0) {
    display->setTextColor(RED);
  } else {
    display->setTextColor(ST77XX_GREEN);
    display->print('+');
  }

  if (abs(current_change) > 1000) {
    display->print(String(current_change, 0));
  } else if (abs(current_change) > 100) {
    display->print(String(current_change, 1));
  } else {
    display->print(String(current_change, 2));
  }

  display->print('%');
  display->setTextColor(WHITE);
}

// Draws the current price of coin on the screen.
void COIN::drawPrice(Adafruit_ST7735 *display, int currency) {
  display->setCursor(PRICE_START_X, PRICE_START_Y);

  String print_price;

  if (current_price / 10000 >= 1) {
    print_price = String(current_price, 1);
  } else if (current_price / 1000 >= 1) {
    print_price = String(current_price, 2);
  } else if (current_price / 100 >= 1) {
    print_price = String(current_price, 3);
  } else if (current_price / 10 >= 1) {
    print_price = String(current_price, 4);
  } else {
    print_price = String(current_price, 5);
  }

  if (currency == 0){ // GBP
    display->print(char(156));
  } else if (currency == 1){ // USD
    display->print(char(36));
  } else if (currency == 2){ // EUR
    display->print(char(237));
  }
  
  display->print(print_price);
}

// Draws a passed bitmap on the screen at the given position with the given
// colour.
void COIN::drawBitmap(Adafruit_ST7735 *tft, int16_t x, int16_t y,
                      const uint8_t *bitmap, int16_t w, int16_t h,
                      uint16_t color) {
  int16_t i, j, byteWidth = (w + 7) / 8;
  uint8_t byte;
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      if (i & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(bitmap + j * byteWidth + i / 8);
      if (byte & 0x80)
        tft->drawPixel(x + i, y + j, color);
    }
  }
}