/*
  ST7735_Menu.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

// ensure this library description is only included once
#ifndef ST7735_Menu_h
#define ST7735_Menu_h

#define BUTTON_HEIGHT 11
#define NORMAL_BUTTON_WIDTH 12
#define EDGE_BORDER 4
#define MAX_SELECTORS 4
#define LAST_BUTTON_X 32
#define FIRST_BUTTON_X 4

#define KEYBOARD_BG_COLOUR 0x0000
#define UNSELECTED_BG_COLOUR 0x8C51
#define SELECTED_BG_COLOUR 0xD69A
#define PROMPT_COLOUR 0xFFFF
#define INPUT_COLOUR 0xF800
#define PROMPT_BACKGROUND 0x0000
#define SELECTED_LETTER_COLOUR 0x0000
#define UNSELECTED_LETTER_COLOUR 0xFFFF
#define MAX_INPUT_LENGTH 32

class Selector {
public:
  Selector(Adafruit_ST7735 *, int, int, char *, char **, int, int, int);
  void display();
  void flashSelected();
  void moveLeft();
  void moveRight();
  void moveDown();
  void moveUp();
  void cycleButtons();
  void press();
  int *getSelected();
  void setSelected(int, int);
  void selectIndex(int);
  void unselectIndex(int);
  void drawItem(int);
  int atTop();
  int atBottom();

  int x;
  int y;
  int value_count;
  char *prompt;
  char **options;
  int window_size;
  int *selected_indexes;
  int max_selected;
  int current_changing_index;

private:
  Adafruit_ST7735 *tft;
  int selected_index;
};

class Button {
public:
  Button(Adafruit_ST7735 *, int, int, int, int, char *, int);
  void addSelector(char *, char **, int, int, int);
  void display();
  void displaySelected();
  void drawSubMenu();
  void subMenuDown();
  void subMenuUp();
  void subMenuLeft();
  void subMenuRight();
  void flashSelectedSelector();

  int x;
  int y;
  int w;
  int h;
  char *action;
  Selector *selectors;
  int selector_count;
  Button *return_button;
  char *pressSubMenu();
  int on_return_button;
  int current_selector;

private:
  Adafruit_ST7735 *tft;
};

// library interface description
class ST7735_Menu {
  // user-accessible "public" interface
public:
  ST7735_Menu(Adafruit_ST7735 *, int, char **);
  char *press();
  void display();
  void moveDown();
  void moveUp();
  Button *getButtons();

  // library-accessible "private" interface
private:
  Adafruit_ST7735 *tft;
  Button *buttons;
  Button *selected;
  int selected_button_index;
  int button_count;
  char **values;
};

#endif
