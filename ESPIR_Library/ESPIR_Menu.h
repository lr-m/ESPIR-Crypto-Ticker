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
#define MAX_SELECTORS 4
#define LAST_BUTTON_X 32
#define FIRST_BUTTON_X 4

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
    Button(Adafruit_ST7735 *, int, int, int, int, char *);
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
    char *pressSubMenu();
    int current_selector;

  private:
    Adafruit_ST7735 *tft;
};

class ESPIR_Menu {
  public:
    ESPIR_Menu(Adafruit_ST7735 *, int, char **);
    char *press();
    void display();
    void moveDown();
    void moveUp();
    Button *getButtons();

  private:
    Adafruit_ST7735 *tft;
    Button *buttons;
    Button *selected;
    int selected_button_index;
    int button_count;
    char **values;
};

#endif
