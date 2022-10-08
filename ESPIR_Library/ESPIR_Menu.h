/*
  ESPIR_Menu.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <math.h>

#include "Colours.h"

#ifndef ESPIR_Menu_h
#define ESPIR_Menu_h

#define BUTTON_HEIGHT 11
#define NORMAL_BUTTON_WIDTH 12
#define EDGE_BORDER 4
#define MAX_ELEMENTS 4
#define LAST_BUTTON_X 32
#define FIRST_BUTTON_X 4

#define SELECTOR_ID 0
#define BUTTON_ID 1

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
    Button(Adafruit_ST7735 *, int, int, int, int, char *, int, int);
    void addSelector(char *, char **, int, int, int);
    void display();
    void displaySelected();
    void drawSubMenu();
    void subMenuDown();
    void subMenuUp();
    void subMenuLeft();
    void subMenuRight();
    void flashSelectedSelector();
    void addButton(char*, int, int);

    int x;
    int y;
    int w;
    int h;
    char *action;
    Selector *selectors;
    Button* buttons;
    int element_count;
    int selector_count;
    int button_count;
    char *pressSubMenu();
    int current_element;
    char* element_type;

  private:
    Adafruit_ST7735 *tft;
};

class ESPIR_Menu {
  public:
    ESPIR_Menu(Adafruit_ST7735 *, int);
    char *press();
    void display();
    void moveDown();
    void moveUp();
    Button *getButtons();
    void addButton(char*, int , int);

  private:
    Adafruit_ST7735 *tft;
    Button *buttons;
    Button *selected;
    int selected_button_index;
    int button_count;
    char **values;
    int buttons_added;
};

#endif
