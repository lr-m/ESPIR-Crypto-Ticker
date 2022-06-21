/*
  ST7735_Coin_Changer.cpp - Coin changer interface for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ST7735_Coin_Changer.h"
#include "HardwareSerial.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Colours.h>

// Constructor for Portfolio Editor
ST7735_Coin_Changer::ST7735_Coin_Changer(Adafruit_ST7735 *display,
                                         char **code_list, COIN *coin_arr,
                                         ST7735_Keyboard *keybrd,
                                         const unsigned char *default_bit) {
  tft = display;
  stage = 0;
  coins = coin_arr;
  current_replacing_index = 0;
  active = 0;
  keyboard = keybrd;
  verified_id = 0;
  coin_code_list = code_list;
  default_bitmap = default_bit;

  selected_picker = 0;

  pickers =
      (Colour_Picker_Component *)malloc(sizeof(Colour_Picker_Component) * 3);
  pickers[0] = Colour_Picker_Component(tft, 5, 10, 0);
  pickers[1] = Colour_Picker_Component(tft, 58, 10, 1);
  pickers[2] = Colour_Picker_Component(tft, 111, 10, 2);
}

// Indicate if the coin changer is active
int ST7735_Coin_Changer::isActive() { return active; }

// Set the coin changer to active
void ST7735_Coin_Changer::setActive() { active = 1; }

// Exit the coin changer
void ST7735_Coin_Changer::exit() { active = 0; }

// Indicate that verification of the entered id was successful
void ST7735_Coin_Changer::verificationSuccess() {
  verified_id = 1;
  tft->setCursor(0, 30);
  tft->setTextColor(ST77XX_GREEN);
  tft->print("Coin ID verified");
  delay(2000);
  stage++;
  display();
}

// Indicate that the verification of the entered id was a failure
void ST7735_Coin_Changer::verificationFailed() {
  tft->setCursor(0, 30);
  tft->setTextColor(RED);
  tft->print("Coin ID not verified\nTry again");
  delay(2000);
  keyboard->reset();
  keyboard->displayPrompt("Enter coin id:");
}

// Draw the square indicating the currently selected colour
void ST7735_Coin_Changer::drawColour() {
  tft->fillRoundRect(50, 76, 60, 25, 4,
                     rgb_to_bgr(pickers[0].getValue(), pickers[1].getValue(),
                                pickers[2].getValue()));
}

// Draw the coin label as selected, grey background
void ST7735_Coin_Changer::drawCoinSelected(int index) {
  tft->setTextSize(1);
  tft->fillRoundRect(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 3,
                     map(floor(index / 3), 0, 5, 5, tft->height() / 2) + 18,
                     0.85 * (tft->width() / 3), 11, 2, DARK_GREY);
  tft->setCursor(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 7,
                 map(floor(index / 3), 0, 5, 5, tft->height() / 2) + 20);
  tft->print(coins[index].coin_code);
}

// Draw the coin label as unselected, black background
void ST7735_Coin_Changer::drawCoinUnselected(int index) {
  tft->setTextSize(1);
  tft->fillRoundRect(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 3,
                     map(floor(index / 3), 0, 5, 5, tft->height() / 2) + 18,
                     0.85 * (tft->width() / 3), 11, 2, BLACK);
  tft->setCursor(map(index % 3, 0, 2, 2, 2 * tft->width() / 3) + 7,
                 map(floor(index / 3), 0, 5, 5, tft->height() / 2) + 20);
  tft->print(coins[index].coin_code);
}

// Interact with the coin changer
int ST7735_Coin_Changer::interact(uint32_t *ir_data) {
  if (*ir_data == 0xF20DFF00) {
    stage = 0;
    active = 0;
    verified_id = 0;
    selected_picker = 0;
    keyboard->reset();
    current_replacing_index = 0;
    return -3;
  }

  if (stage == 0) { // Select coin to replace
    drawCoinUnselected(current_replacing_index);

    // left
    if (*ir_data == 0xF708FF00) {
      drawCoinUnselected(current_replacing_index);
      current_replacing_index = current_replacing_index == 0
                                    ? COIN_COUNT - 1
                                    : current_replacing_index - 1;
      drawCoinSelected(current_replacing_index);
    }

    // right
    if (*ir_data == 0xA55AFF00) {
      drawCoinUnselected(current_replacing_index);
      current_replacing_index = current_replacing_index == COIN_COUNT - 1
                                    ? 0
                                    : current_replacing_index + 1;
      drawCoinSelected(current_replacing_index);
    }

    // down
    if (*ir_data == 0xAD52FF00) {
      drawCoinUnselected(current_replacing_index);
      current_replacing_index = current_replacing_index + 3 >= COIN_COUNT
                                    ? current_replacing_index
                                    : current_replacing_index + 3;
      drawCoinSelected(current_replacing_index);
    }

    // up
    if (*ir_data == 0xE718FF00) {
      drawCoinUnselected(current_replacing_index);
      current_replacing_index = current_replacing_index - 3 < 0
                                    ? current_replacing_index
                                    : current_replacing_index - 3;
      drawCoinSelected(current_replacing_index);
    }

    // ok
    if (*ir_data == 0xE31CFF00) {
      stage++;
      display();
    }
  } else if (stage == 1) { // Search for coin id
    keyboard->interact(ir_data);
    if (keyboard->enterPressed() == 1) {
      // Indicate SSID and pwd loaded into EEPROM
      int i = 0;
      char *keyboard_val = keyboard->getCurrentInput();

      while (keyboard_val[i] != 0) {
        loaded_id[i] = keyboard_val[i];
        i++;
      }
      loaded_id[i] = 0;

      keyboard->reset();

      return -2;
    }
  } else if (stage == 2) { // Enter coin code
    keyboard->interact(ir_data);
    if (keyboard->enterPressed() == 1) {
      // Indicate SSID and pwd loaded into EEPROM
      int i = 0;
      char *keyboard_val = keyboard->getCurrentInput();

      while (keyboard_val[i] != 0) {
        loaded_code[i] = keyboard_val[i];
        i++;
      }
      loaded_code[i] = 0;

      stage++;
      display();
    }
  } else if (stage == 3) { // Select coin colour
    // ok
    if (*ir_data == 0xE31CFF00) {
      loadIntoSelectedCoin();
      stage = 0;
      active = 0;
      verified_id = 0;
      selected_picker = 0;
      keyboard->end();
      return current_replacing_index;
    }

    // left
    if (*ir_data == 0xF708FF00) {
      pickers[selected_picker].display();
      selected_picker = selected_picker == 0 ? 2 : selected_picker - 1;
      pickers[selected_picker].displaySelected();
    }

    // right
    if (*ir_data == 0xA55AFF00) {
      pickers[selected_picker].display();
      selected_picker = selected_picker == 2 ? 0 : selected_picker + 1;
      pickers[selected_picker].displaySelected();
    }

    // up
    if (*ir_data == 0xE718FF00) {
      pickers[selected_picker].incrementValue();
      drawColour();
    }

    // down
    if (*ir_data == 0xAD52FF00) {
      pickers[selected_picker].decrementValue();
      drawColour();
    }
  }

  return -1;
}

// Load the entered details into the coin with the selected ID
void ST7735_Coin_Changer::loadIntoSelectedCoin() {
  coins[current_replacing_index].bitmap_present = 0;
  
  int i = 0;
  while(loaded_code[i] != 0){
    coins[current_replacing_index].coin_code[i] = loaded_code[i];
    i++;
  }
  coins[current_replacing_index].coin_code[i] = 0;

  i = 0;
  while(loaded_id[i] != 0){
     coins[current_replacing_index].coin_id[i] = loaded_id[i];
    i++;
  }
  coins[current_replacing_index].coin_id[i] = 0;

  coins[current_replacing_index].portfolio_colour = rgb_to_bgr(
      pickers[0].getValue(), pickers[1].getValue(), pickers[2].getValue());

  coins[current_replacing_index].bitmap = default_bitmap;

  if (coins[current_replacing_index].candles_init == 1)
    coins[current_replacing_index].candles->reset();

  coin_code_list[current_replacing_index] = coins[current_replacing_index].coin_code;
}

// Display the coin changer interface on screen
void ST7735_Coin_Changer::display() {
  if (stage == 0) { // Choose coin to replace
    tft->fillRect(0, 0, tft->width(), tft->height(), BLACK);
    tft->setCursor(5, 5);
    tft->setTextColor(WHITE);
    tft->print("Select coin to replace:");

    int i = 0;
    while (i < COIN_COUNT) {
      tft->fillRoundRect(map(i % 3, 0, 2, 2, 2 * tft->width() / 3),
                         map(floor(i / 3), 0, 5, 5, tft->height() / 2) + 18, 3,
                         11, 2, coins[i].portfolio_colour);
      tft->setCursor(map(i % 3, 0, 2, 2, 2 * tft->width() / 3) + 7,
                     map(floor(i / 3), 0, 5, 5, tft->height() / 2) + 20);
      tft->print(coins[i].coin_code);
      i++;
    }

    drawCoinSelected(current_replacing_index);
  } else if (stage == 1) { // Search for the coin on coingecko
    keyboard->displayPrompt("Enter id (e.g bitcoin):");
    keyboard->display();
  } else if (stage == 2) { // Search for the coin on coingecko
    keyboard->displayPrompt("Enter code (e.g. BTC):");
    keyboard->display();
  } else if (stage == 3) { // Set the coin properties (colour)
    tft->fillRect(0, 0, tft->width(), tft->height(), BLACK);
    tft->setTextColor(WHITE);
    tft->setCursor(38, 60);
    tft->print("Current Colour:");

    drawColour();

    for (int i = 0; i < 3; i++) {
      if (i == selected_picker) {
        pickers[i].displaySelected();
      } else {
        pickers[i].display();
      }
    }
  }
}

// Converts rgb to 5:6:5 bgr (accepted by TFT display)
int ST7735_Coin_Changer::rgb_to_bgr(unsigned char r, unsigned char g,
                                    unsigned char b) {
  if (r < 0 || 255 < r || g < 0 || 255 < g || b < 0 || b > 255)
    return -1;

  unsigned char red = r >> 3;
  unsigned char green = g >> 2;
  unsigned char blue = b >> 3;

  int result = (blue << (5 + 6)) | (green << 5) | red;
  //int result = (red << (5 + 6)) | (green << 5) | blue;

  return result;
}

// Constuctor for Colour_Picker
Colour_Picker_Component::Colour_Picker_Component(Adafruit_ST7735 *display,
                                                 int x_co, int y_co, int col) {
  tft = display;
  y = y_co;
  colour = col;
  x = x_co;
  value = 255;
}

// Display the picker normally
void Colour_Picker_Component::display() {
  tft->setTextSize(1);
  tft->setTextColor(WHITE);
  tft->fillRoundRect(x, y, 46, 32, 3, BLACK);
  tft->setCursor(x + 5, y + 2);

  if (colour == 0) {
    tft->println("Red:");
    tft->setTextColor(RED);
  } else if (colour == 1) {
    tft->println("Green:");
    tft->setTextColor(ST77XX_GREEN);
  } else if (colour == 2) {
    tft->println("Blue:");
    tft->setTextColor(BLUE);
  }

  tft->setTextSize(2);
  tft->setCursor(x + 5, y + 15);
  tft->println(value);
  tft->setTextSize(1);
}

// Display the picker as selected
void Colour_Picker_Component::displaySelected() {
  tft->setTextSize(1);
  tft->setTextColor(BLACK);
  tft->fillRoundRect(x, y, 46, 32, 3, GRAY);
  tft->setCursor(x + 5, y + 2);

  if (colour == 0) {
    tft->println("Red:");
    tft->setTextColor(RED);
  } else if (colour == 1) {
    tft->println("Green:");
    tft->setTextColor(ST77XX_GREEN);
  } else if (colour == 2) {
    tft->println("Blue:");
    tft->setTextColor(BLUE);
  }

  tft->setTextSize(2);
  tft->setCursor(x + 5, y + 15);
  tft->println(value);
  tft->setTextSize(1);
}

// Get the current value selected
unsigned char Colour_Picker_Component::getValue() { return value; }

// Increase the value by the amount selected by the user
void Colour_Picker_Component::incrementValue() {
  tft->fillRoundRect(x, y + 10, 46, 22, 3, GRAY);

  value = value < 245 ? value + 10 : 255;

  if (colour == 0) {
    tft->setTextColor(RED);
  } else if (colour == 1) {
    tft->setTextColor(ST77XX_GREEN);
  } else if (colour == 2) {
    tft->setTextColor(BLUE);
  }

  tft->setTextSize(2);
  tft->setCursor(x + 5, y + 15);
  tft->println(value);
  tft->setTextSize(1);

  tft->setTextColor(WHITE);
}

// Decrease the value by the amount selected by the user
void Colour_Picker_Component::decrementValue() {
  tft->fillRoundRect(x, y + 10, 46, 22, 3, GRAY);

  value = value > 15 ? value - 10 : 5;

  if (colour == 0) {
    tft->setTextColor(RED);
  } else if (colour == 1) {
    tft->setTextColor(ST77XX_GREEN);
  } else if (colour == 2) {
    tft->setTextColor(BLUE);
  }

  tft->setTextSize(2);
  tft->setCursor(x + 5, y + 15);
  tft->println(value);
  tft->setTextSize(1);

  tft->setTextColor(WHITE);
}