#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <ArduinoJson.h>
#include <Colours.h>
#include <IR_Codes.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <IRremote.h>
#include <SPI.h>
#include <ESPIR_Candle_Graph.h>
#include <ESPIR_Coin_Changer.h>
#include <ESPIR_Keyboard.h>
#include <ESPIR_Menu.h>
#include <ESPIR_Portfolio.h>
#include <ESPIR_Portfolio_Editor.h>

extern unsigned char epd_bitmap_logo_with_name[];
extern unsigned char epd_bitmap_logo_red[];
extern unsigned char epd_bitmap_logo_green[];

// Coin bitmaps
extern unsigned char BTC_logo[];
extern unsigned char ETH_logo[];
extern unsigned char USDT_logo[];
extern unsigned char ADA_logo[];
extern unsigned char BNB_logo[];
extern unsigned char XRP_logo[];
extern unsigned char SOL_logo[];
extern unsigned char DOT_logo[];
extern unsigned char DOGE_logo[];
extern unsigned char LTC_logo[];
extern unsigned char AVAX_logo[];
extern unsigned char ALGO_logo[];
extern unsigned char XMR_logo[];
extern unsigned char MATIC_logo[];
extern unsigned char TRON_logo[];
extern unsigned char ETC_logo[];
extern unsigned char BAT_logo[];
extern unsigned char USDC_logo[];
extern unsigned char BUSD_logo[];
extern unsigned char SHIB_logo[];
extern unsigned char DAI_logo[];

// Define PIN config
#define TFT_SCL   D1
#define TFT_SDA   D2
#define TFT_RES   D3
#define TFT_DC    D4
#define RECV_PIN  D5
#define TFT_CS    D6

// OLD PIN CONFIG
//#define TFT_SCL   D1
//#define TFT_SDA   D2
//#define TFT_RES   D3
//#define RECV_PIN  D4
//#define TFT_DC    D5
//#define TFT_CS    D6

#define UPDATE_INTERVAL 30 // Update every 30 seconds

#define MAX_SELECTED_COINS 8 // Maximum of 8 coins in coin list

#define COIN_MENU_BUTTON_COUNT 8

#define SETTINGS_START_ADDRESS 64 // Where settings are saved in EEPROM

// Additional coins save details
#define COIN_COUNT_ADDRESS 105
#define COIN_START_ADDRESS 106
#define COIN_BLOCK_SIZE 45
#define ADDED_COINS_LIMIT 5

// Portfolio save details
#define PORTFOLIO_BLOCK_SIZE 20
#define PORTFOLIO_START_ADDRESS 332
#define PORTFOLIO_COUNT_ADDRESS 331
#define PORTFOLIO_LIMIT 9

#define EEPROM_SIZE 512

int coingecko_up = 1;

char *stack_start;

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft =
    Adafruit_ST7735(TFT_CS, TFT_DC, TFT_SDA, TFT_SCL, TFT_RES);

// For interface
int selected_coins_count = 0;   // Current number of coins selected
int current_coin = 0;               // Index of current coin being displayed
int current_second;  // Current second from time NTP server
long int second_offset_counter = 0;
int current_minute;
long int minute_offset_counter = 0;

// Flags
int ssid_entered;            // Indicates if SSID of network entered
int password_entered;        // Indicates if password of network entered
int network_initialised = 0; // Indicates if network has been initialised
int eeprom_address = 0;      // Current address being written to on EEPROM
int in_menu = 0;             // Indicates if currently in the menu
int in_crypto_settings = 0;         // Indicates if currently in settings submenu
int in_portfolio_settings = 0;         // Indicates if currently in settings submenu
int in_coinlist = 0;         // Indicates if currently in coin list submenu
int in_portfolio_editor = 0;
int coin_time_slot_moved = 1; // Indicates if coin candles have been moved along
int portfolio_time_slot_moved =
    0; // Indicates if portfolio candles have been moved along
int data_refreshed = 1;
int portfolio_cycle_disabled = 1;

// For HTTP connection
const char *url_start = "https://api.coingecko.com/api/v3/simple/price?ids=";
const char *url_end = "&include_24hr_change=true&vs_currencies=";

// IR remote
IRrecv irrecv(RECV_PIN);
decode_results results;

// Instantiate keyboard
ESPIR_Keyboard *keyboard;

// Instantiate menu
ESPIR_Menu *coin_menu;
char **coin_menu_button_functions;

char *coin_change_times[5] = {"Off", "5", "15", "30", "60"};
int coin_change_times_values[5] = {0, 5, 15, 30, 60};

char *candle_change_times[4] = {"5", "15", "30", "60"};
int candle_change_times_values[4] = {5, 15, 30, 60};

char *bitmap_toggle[2] = {"Enabled", "Disabled"};

char *currency_options[3] = {"GBP", "USD", "EUR"};
char *currency_options_lower[3] = {"gbp", "usd", "eur"};
char *currency_options_changes[3] = {"gbp_24h_change", "usd_24h_change", "eur_24h_change"};

// Instantiate components of Portfolio
ESPIR_Portfolio_Editor *portfolio_editor;
ESPIR_Portfolio *portfolio;
ESPIR_Coin_Changer *coin_changer;
ESPIR_Value_Drawer *value_drawer;

char **coin_list;             // List of all coin codes
int coin_cycle_delay;         // Delay between coin changes
double next_coin_change = -1;    // Allows temporary extension of coin change delay
int coin_candle_update_delay; // How often candles are pushed along (mins)
int last_coin_candle_update_delay =
    5;                             // To detect changes in candle update time
int portfolio_candle_update_delay; // How often candles are pushed along (mins)
int last_portfolio_candle_update_delay =
    5; // To detect changes in candle update time
int last_bitmap_toggle = 0;

int mode = 1; // Coin or portfolio mode
int last_currency = 0; // Checks for currency changes

int display_all_start_index = 0;

COIN *coins;           // List of coins that can be selected
COIN **selected_coins; // List of pointers to coins currently selected

char* request_url;

void setup(void) { 
  Serial.begin(115200);

  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  irrecv.enableIRIn(); // Enable IR reciever

  request_url = (char*) malloc(sizeof(char) * 450);

  // Initialise display
  //tft.initR(INITR_GREENTAB); // Init ST7735S chip
  tft.initR(INITR_BLACKTAB); // Init ST7735S chip
  tft.setRotation(1);

  // Initialise keyboard
  keyboard = (ESPIR_Keyboard *)malloc(sizeof(ESPIR_Keyboard));
  *keyboard = ESPIR_Keyboard(&tft);

  // Initialise coin menu
  coin_menu_button_functions =
      (char **)malloc(sizeof(char *) * COIN_MENU_BUTTON_COUNT);

  coin_menu_button_functions[0] = "Select Coins";
  coin_menu_button_functions[1] = "Edit Portfolio";
  coin_menu_button_functions[2] = "Add New Coin";
  coin_menu_button_functions[3] = "Crypto Display Settings";
  coin_menu_button_functions[4] = "Portfolio Settings";
  coin_menu_button_functions[5] = "Clear WiFi Credentials";
  coin_menu_button_functions[6] = "Reset Coins (Restart)";
  coin_menu_button_functions[7] = "Clear Portfolio";
  coin_menu_button_functions[8] = "Exit Menu";
  coin_menu = (ESPIR_Menu *)malloc(sizeof(ESPIR_Menu));
  *coin_menu =
      ESPIR_Menu(&tft, COIN_MENU_BUTTON_COUNT, coin_menu_button_functions);

  // For drawing prices and percentage changes
  value_drawer = (ESPIR_Value_Drawer *)malloc(sizeof(ESPIR_Value_Drawer));
  *value_drawer = ESPIR_Value_Drawer(&tft);

  // Create coin array
  coins = (COIN *)malloc(sizeof(COIN) * COIN_COUNT);
  selected_coins = (COIN **)malloc(sizeof(COIN *) * MAX_SELECTED_COINS);

  coin_list = (char **)malloc(sizeof(char *) * COIN_COUNT);

  // Background col, bitmap col, portfolio col
  coins[0] = 
    COIN("BTC", "bitcoin", BTC_logo, GOLD, WHITE, GOLD, 0, value_drawer);
  coins[1] = 
    COIN("ETH", "ethereum", ETH_logo, VIOLET, GRAY, VIOLET, 0, value_drawer);
  coins[2] =
    COIN("USDT", "tether", USDT_logo, GREEN, WHITE, GREEN, 0, value_drawer);
  coins[3] =
    COIN("USDC", "usd-coin", USDC_logo, DARK_BLUE, WHITE, DARK_BLUE, 0, value_drawer);
  coins[4] = 
    COIN("BNB", "binancecoin", BNB_logo, WHITE, GOLD, GOLD, 0, value_drawer);
  coins[5] = 
    COIN("XRP", "ripple", XRP_logo, DARK_GREY, WHITE, DARK_GREY, 0, value_drawer);
  coins[6] = 
    COIN("ADA", "cardano", ADA_logo, DARK_BLUE, WHITE, DARK_BLUE, 0, value_drawer);
  coins[7] = 
    COIN("BUSD", "binance-usd", BUSD_logo, WHITE, GOLD, GOLD, 0, value_drawer);
  coins[8] =
    COIN("SOL", "solana", SOL_logo, PINK, LIGHTNING_BLUE, PINK, 0, value_drawer);
  coins[9] = 
    COIN("DOT", "polkadot", DOT_logo, WHITE, BLACK, WHITE, 0, value_drawer);
  coins[10] = 
    COIN("DOGE", "dogecoin", DOGE_logo, GOLD, WHITE, GOLD, 0, value_drawer);
  coins[11] =
    COIN("MATIC", "matic-network", MATIC_logo, PURPLE, WHITE, PURPLE, 0, value_drawer);
  coins[12] =
    COIN("SHIB", "shiba-inu", SHIB_logo, RED, ORANGE, ORANGE, 0, value_drawer);
  coins[13] = 
    COIN("AVAX", "avalanche-2", AVAX_logo, RED, WHITE, RED, 0, value_drawer);
  coins[14] = 
    COIN("DAI", "dai", DAI_logo, GOLD, WHITE, GOLD, 0, value_drawer);
  coins[15] = 
    COIN("TRX", "tron", TRON_logo, RED, WHITE, RED, 0, value_drawer);
  coins[16] = 
    COIN("ETC", "ethereum-classic", ETC_logo, WHITE, ETC_GREEN, ETC_GREEN, 0, value_drawer);
  coins[17] =
    COIN("LTC", "litecoin", LTC_logo, DARK_GREY, WHITE, DARK_GREY, 0, value_drawer);
  coins[18] =
    COIN("XMR", "monero", XMR_logo, ORANGE, DARK_GREY, ORANGE, 0, value_drawer);
  coins[19] =
    COIN("ALGO", "algorand", ALGO_logo, WHITE, BLACK, WHITE, 0, value_drawer);
  coins[20] =
    COIN("BAT", "basic-attention-token", BAT_logo, WHITE, SALMON, SALMON, 0, value_drawer);

  // Add selectors to respective submenus
  coin_menu->getButtons()[0].addSelector("Select up to 8 coins:", coin_list, 3,
                                         MAX_SELECTED_COINS, COIN_COUNT);
  
  coin_menu->getButtons()[3].addSelector(
      "Cycle duration (secs):", coin_change_times, 5, 1, 5);
  coin_menu->getButtons()[3].addSelector(
      "Candle duration (mins):", candle_change_times, 4, 1, 4);
  coin_menu->getButtons()[3].addSelector(
      "Currency:", currency_options, 3, 1, 3);
  coin_menu->getButtons()[3].addSelector(
      "Toggle bitmaps:", bitmap_toggle, 2, 1, 2);

  coin_menu->getButtons()[4].addSelector(
      "Candle duration (mins):", candle_change_times, 4, 1, 4);
  coin_menu->getButtons()[4].addSelector(
      "Portfolio display cycle:", bitmap_toggle, 2, 1, 2);
  
  for (int i = 0; i < COIN_COUNT; i++)
    coin_list[i] = coins[i].coin_code;

  // Initialise portfolio
  portfolio_editor =
      (ESPIR_Portfolio_Editor *)malloc(sizeof(ESPIR_Portfolio_Editor));
  *portfolio_editor = ESPIR_Portfolio_Editor(&tft, coins);

  portfolio = (ESPIR_Portfolio *)malloc(sizeof(ESPIR_Portfolio));
  *portfolio = ESPIR_Portfolio(&tft, portfolio_editor, coins, value_drawer);

  coin_changer = (ESPIR_Coin_Changer *)malloc(sizeof(ESPIR_Coin_Changer));
  *coin_changer = ESPIR_Coin_Changer(&tft, coin_list, coins, keyboard);

  // Try to read network credentials from EEPROM if they exist
  EEPROM.begin(EEPROM_SIZE);
  byte value = EEPROM.read(0);

  printEEPROM();

  // First ever boot has random memory, need to clear
  if (value != 1 && value != 0){
    clearEEPROM();
  }

  // Check if credits have been written, take necessary action
  if (value == 0) {
    ssid_entered = 0;
    password_entered = 0;
    keyboard->setModeClear(0, 14);
    keyboard->displayPrompt("Enter Network Name (SSID):");
    keyboard->display();
    keyboard->setInputLengthLimit(30);
  } else if (value == 1) {
    ssid_entered = 1;
    password_entered = 1;
  }

  loadSettingsFromEEPROM();

  // Get default coin delay time from selector
  coin_cycle_delay = coin_change_times_values[coin_menu->getButtons()[3].selectors[0].getSelected()[0]];

  // Get default update times from selector
  coin_candle_update_delay = candle_change_times_values
      [coin_menu->getButtons()[3].selectors[1].getSelected()[0]];
  last_coin_candle_update_delay = coin_candle_update_delay;
  
  portfolio_candle_update_delay = candle_change_times_values
      [coin_menu->getButtons()[4].selectors[0].getSelected()[0]];
  last_portfolio_candle_update_delay = portfolio_candle_update_delay;

  last_bitmap_toggle = coin_menu -> getButtons()[3].selectors[3].getSelected()[0];

  portfolio_cycle_disabled = coin_menu->getButtons()[4].selectors[1].getSelected()[0];

  last_currency = coin_menu -> getButtons()[3].selectors[2].getSelected()[0];

  if (last_bitmap_toggle == 1){
    for (int i = 0; i < COIN_COUNT; i++){
      coins[i].toggleBitmap();
    }
  }
  
  readCoinsFromEEPROM();
  loadPortfolioFromEEPROM();
}

void loop() {
  updateTime(); // Update time variables

//  Serial.print(millis());
//  Serial.print(" - Heap: ");
//  Serial.print(ESP.getFreeHeap());
//  Serial.print(", Block Size: ");
//  Serial.println(ESP.getMaxFreeBlockSize());

  if (ssid_entered == 0) { // Get network name
    if (keyboard->enterPressed() == 1) {
      // Indicate SSID and pwd loaded into EEPROM
      EEPROM.write(eeprom_address, 1);
      eeprom_address++;
      writeToEEPROM(keyboard->getCurrentInput());

      ssid_entered = 1;

      keyboard->reset();
      keyboard->setModeClear(0, 14);
      keyboard->displayPrompt("Enter Network Password:");
      keyboard->display();
    } else if (irrecv.decode()) {
      keyboard->interact(&irrecv.decodedIRData.decodedRawData);
      irrecv.resume();
    }
  } else if (password_entered == 0) { // Get network password
    if (keyboard->enterPressed() == 1) {
      // Write password to EEPROM
      writeToEEPROM(keyboard->getCurrentInput());

      password_entered = 1;
      keyboard->reset();
    } else if (irrecv.decode()) {
      keyboard->interact(&irrecv.decodedIRData.decodedRawData);
      irrecv.resume();
    }

    // Commit changes to EEPROM
    EEPROM.commit();
  } else if (network_initialised == 0) { // Initialise network
    // Load the credentials from EEPROM and init network connection
    char loaded_ssid[31];
    char loaded_password[31];

    drawIntroAnimation();

    loadCredsFromEEPROM(loaded_ssid, loaded_password);

    initialiseNetwork(loaded_ssid, loaded_password);
    
    // Initialise the candle for first coin (bitcoin) if none recovered from storage
    if (selected_coins_count == 0){
      selected_coins[0] = coins;
      selected_coins[0] -> initCandles(&tft);
      selected_coins_count++;
    }

    // Data fetching time affordance
    drawFetchingData();

    network_initialised = 1; // Indicate successful network init
  } else if (in_menu == 1) { // Do menu operations
    if (in_coinlist == 1) {  // Interact with coin list submenu
      interactWithCoinList();
      return;
    }

    if (in_crypto_settings == 1) { // Interact with settings submenu
      interactWithCryptoSettings();
      return;
    }

    if (in_portfolio_settings == 1) { // Interact with settings submenu
      interactWithPortfolioSettings();
      return;
    }

    if (coin_changer->isActive() == 1) { // Interact with coin changer
      if (irrecv.decode()) {
        int response =
            coin_changer->interact(&irrecv.decodedIRData.decodedRawData);
        if (response >= 0) { // Successful
          tft.fillScreen(BLACK);
          drawFetchingData();
          
          // If overwritten coin was in portfolio, reset the EEPROM candles and set coin amount to 0
          if (coins[response].amount != 0){
            coins[response].amount = 0;
            portfolio->clearCandles();

            // Get data if portfolio not empty
            if (portfolio_editor->selected_portfolio_indexes[0] != -1)
              getData(2); // Get data for portfolio
            
            portfolio->addPriceToCandles();
          }
          
          writeCoinToEEPROM(coins+response, response);
          resetCoins();

          drawMenu();
        } else if (response == -2) { // Perform verification
          if (verifyID(coin_changer->loaded_id) == 1) {
            // Ensure not adding a coin that already exists
            for (int i = 0; i < COIN_COUNT; i++){
              if (strcmp(coin_changer->loaded_id, coins[i].coin_id) == 0){
                coin_changer->duplicateDetected();
                irrecv.resume();
                return;
              }
            }

            // Indicate successful verification
            coin_changer->verificationSuccess();
          } else {
            coin_changer->verificationFailed();
          }
        } else if (response == -3){ // User cancelled
          drawMenu();
        }
        irrecv.resume();
      }
      return;
    }

    if (portfolio_editor->active == 1) { // Interact with portfolio editor
      if (irrecv.decode()) {
        if (portfolio_editor->interact(&irrecv.decodedIRData.decodedRawData) ==
            0) {
          drawMenu();
          if (portfolio_editor->checkForChange() == 1)
            portfolio->clearCandles();
        }

        irrecv.resume();
      }
      return;
    }

    interactWithMenu();
  } else { // Display crypto interface   
    if (network_initialised == 1){
      
      getData(1); // Get data for coin

      // Get data if portfolio not empty
      if (portfolio_editor->selected_portfolio_indexes[0] != -1){
        getData(2); // Get data for portfolio
      }
      
      network_initialised++;
    }

    // IR receive loop
    for (int i = 0; i < 10; i++) {
      delay(50);
      if (irrecv.decode()) {
        // Check if 'OK' pressed to open menu
        if (irrecv.decodedIRData.decodedRawData == IR_OK){
          in_menu = 1;
          drawMenu();
          delay(250);
          irrecv.resume();
          return;
        }

        // Change mode if up or down pressed
        if (irrecv.decodedIRData.decodedRawData == IR_UP ||
            irrecv.decodedIRData.decodedRawData == IR_DOWN){

          // Increase mode 
          if (irrecv.decodedIRData.decodedRawData == IR_UP){
            mode++;
            if (mode > 3)
              mode = 1;
          }

          // Decrease mode
          if (irrecv.decodedIRData.decodedRawData == IR_DOWN){
            mode--;
            if (mode < 1)
              mode = 3;
          }
          
          irrecv.resume();

          if (mode == 1) {
            selected_coins[current_coin]->display(&tft, 
              coin_menu->getButtons()[3].selectors[2].getSelected()[0]);
            incrementCoinCycleDelay();
          } else if (mode == 2) {
            portfolio->display(coin_menu->getButtons()[3].selectors[2].getSelected()[0]);
          } else {
            displayAllCoins();
          }

          break;
        }

        // Check if user wants to move to next/previous coin
        if (mode == 1) {
          // Previous coin
          if (irrecv.decodedIRData.decodedRawData == IR_LEFT)
            displayPreviousCoin();

          // Next coin
          if (irrecv.decodedIRData.decodedRawData == IR_RIGHT)
            displayNextCoin();
        } else if (mode == 2) {
          if (irrecv.decodedIRData.decodedRawData == IR_LEFT) { // Previous mode
            portfolio->previousMode();
            portfolio->display(coin_menu->getButtons()[3].selectors[2].getSelected()[0]);
          } else if (irrecv.decodedIRData.decodedRawData == IR_RIGHT) { // Next mode
            portfolio->nextMode();
            portfolio->display(coin_menu->getButtons()[3].selectors[2].getSelected()[0]);
          }
        } else if (mode == 3 && selected_coins_count > 4){ // Check that multiple pages filled
          // Invert currently shown coins (left/right, max 2 pages so both identical result)
          if (irrecv.decodedIRData.decodedRawData == IR_LEFT || 
              irrecv.decodedIRData.decodedRawData == IR_RIGHT) { 
            display_all_start_index += 4;
            if (display_all_start_index >= MAX_SELECTED_COINS){
              display_all_start_index = 0;
            }
            displayAllCoins();
          }
        }

        irrecv.resume();
      }
    }

    if (next_coin_change == -1)
      displayNextCoin();

    // Move coin data along every 'coin_candle_update_delay' minutes
    if (current_minute % coin_candle_update_delay == 0 &&
        coin_time_slot_moved == 0) {
      for (int i = 0; i < selected_coins_count; i++)
        selected_coins[i]->candles->nextTimePeriod(
            selected_coins[i]->current_price);

      coin_time_slot_moved = 1;
    } else if (current_minute % 5 != 0 && coin_time_slot_moved == 1) {
      coin_time_slot_moved = 0;
    }

    // Move portfolio data along every 'portfolio_candle_update_delay' minutes
    if (current_minute % portfolio_candle_update_delay == 0 &&
        portfolio_time_slot_moved == 0) {
      portfolio->nextTimePeriod();

      portfolio_time_slot_moved = 1;
    } else if (current_minute % 5 != 0 &&
               portfolio_time_slot_moved == 1) {
      portfolio_time_slot_moved = 0;
    }

    // Update coin and portfolio data every 30 seconds
    if (current_second % UPDATE_INTERVAL <= 5 && data_refreshed == 0) {
      // Get individual coin data
      getData(1);
      portfolio->refreshSelectedCoins(); // Add any new coins to selected
      
      // Get data if portfolio not empty
      if (portfolio_editor->selected_portfolio_indexes[0] != -1){
        getData(2); // Get data for portfolio
      }
      
      portfolio->addPriceToCandles();

      data_refreshed = 1;

      // If coin cycle disabled, redraw when data updated
      if (mode == 1 && coin_cycle_delay == 0){
        selected_coins[current_coin]->display(&tft, coin_menu->getButtons()[3].selectors[2].getSelected()[0]);
      }

      // Redisplay portfolio
      if (mode == 2){
        if (portfolio_cycle_disabled == 0){
          portfolio->nextMode();
        }
        
        portfolio->display(coin_menu->getButtons()[3].selectors[2].getSelected()[0]);
      }

      // Refresh coins if cycle disabled
      if (mode == 3 && coin_cycle_delay == 0) displayAllCoins();
    } else if (current_second % UPDATE_INTERVAL > 5) {
      data_refreshed = 0;
    }

    // Move to next coin every *selected* seconds if in crypto modes
    if (coin_cycle_delay > 0 && millis()/1000 > next_coin_change) {
      if (mode == 1) {
        displayNextCoin();
      } else if (mode == 3){
        display_all_start_index += 4;
        if (display_all_start_index >= MAX_SELECTED_COINS){
          display_all_start_index = 0;
        }
        displayAllCoins();
      }
    }
  }
}

void displayNextCoin(){      
  current_coin++;
  if (current_coin == selected_coins_count)
    current_coin = 0;

  selected_coins[current_coin]->display(&tft, coin_menu->getButtons()[3].selectors[2].getSelected()[0]);

  incrementCoinCycleDelay();
}

void displayPreviousCoin(){
  if (current_coin == 0)
    current_coin = selected_coins_count;

  current_coin--;

  selected_coins[current_coin]->display(&tft, coin_menu->getButtons()[3].selectors[2].getSelected()[0]);

  incrementCoinCycleDelay();
}

void displayAllCoins(){
  tft.fillScreen(BLACK);
  int height = tft.height()/4;
  tft.setTextColor(ST77XX_WHITE);
  
  for (int i = display_all_start_index; i < min(selected_coins_count, display_all_start_index + 4); i++){
    tft.setTextSize(1);
    tft.setCursor(7, (i-display_all_start_index) * height);
    tft.fillRect(2, (i-display_all_start_index) * height, 2, height,
                  selected_coins[i]->portfolio_colour);

    tft.setCursor(7, (i-display_all_start_index) * height + (height/2 - 4));
    tft.print(selected_coins[i]->coin_code);
    
    tft.setCursor(52, (i-display_all_start_index) * height + (height-20)/2);
    tft.setTextColor(WHITE);
    value_drawer->drawPrice(7, selected_coins[i]->current_price, 7, 1, 
      coin_menu -> getButtons()[3].selectors[2].getSelected()[0]); // Currency

    tft.setCursor(52,  (i-display_all_start_index) * height + height/2 + 2);
    value_drawer->drawPercentageChange(5,
      selected_coins[i]->current_change, 3, 1);

    selected_coins[i]->candles->displaySmall(105, tft.width()-105, 
      (i-display_all_start_index) * height + 1, (i-display_all_start_index) * height + height - 2);
  }

  incrementCoinCycleDelay();
}

void incrementCoinCycleDelay(){
  next_coin_change = millis()/1000 + coin_cycle_delay;
}

void drawIntroAnimation() {
  // Give user a chance to wipe stored WiFi details (e.g. if network changes)

  tft.fillScreen(BLACK);
  tft.setCursor(10, 5);
  drawBitmap(0, 0, epd_bitmap_logo_green, 160, 58, LIGHT_GREEN);
  drawBitmap(0, 0, epd_bitmap_logo_red, 160, 58, RED);

  delay(500);

  tft.setTextColor(LIGHT_GREEN);
  tft.setCursor(8, tft.height() - 67);
  tft.write("Powered by CoinGecko API");

  tft.setTextColor(WHITE);
  tft.setCursor(10, tft.height() - 40);
  tft.write("# to wipe EEPROM memory");
  tft.setCursor(10, tft.height() - 30);
  tft.write("* to clear WiFi details");

  // Give user opportunity to clear WiFi credentials without needing to connect
  for (int i = 5; i <= tft.width() - 10; i += 2) {
    delay(50);
    tft.fillRect(i, tft.height() - 15, 3, 10, WHITE);

    if (irrecv.decode()) {
      // Clear WiFi creds
      if (irrecv.decodedIRData.decodedRawData == IR_ASTERISK) {
        clearEEPROMArea(0, SETTINGS_START_ADDRESS);
        ESP.restart();
      }

      // Clear Selected Coins
      if (irrecv.decodedIRData.decodedRawData == 0xBA45FF00) {
        for (int i = 0; i < 10; i++){
          EEPROM.write(SETTINGS_START_ADDRESS+i, 255);
        }
        EEPROM.commit();
        ESP.restart();
      }

      // Wipe EEPROM
      if (irrecv.decodedIRData.decodedRawData == IR_HASHTAG) {
        clearEEPROM();
        ESP.restart();
      }
    }
  }
}

// Data Retrieval

void getData(int app_mode) { 
  // Instanciate Secure HTTP communication
  HTTPClient http;
  http.useHTTP10(true);
  constructURL(app_mode, request_url); // Get the URL

  // Init client
  WiFiClientSecure client;
  client.setInsecure();

  if (app_mode == 1){
   Serial.println("Coin Data Update Attempt...");
  } else {
   Serial.println("Portfolio Data Update Attempt...");
  }

  // HTTP request
  http.begin(client, request_url);
  int httpCode = http.GET();

  // Buffers for data
  StaticJsonDocument<60> doc;
  char id_buffer[35];

  // OK http response
  if (httpCode == 200) {    
     int selected_currency = coin_menu -> getButtons()[3].selectors[2].getSelected()[0];
     int read_bytes;

     // Parse incoming JSON from stream
     http.getStream().find("\"");
     do {
        read_bytes = http.getStream().readBytesUntil('"', id_buffer, 32);
        id_buffer[read_bytes] = 0;
        
        http.getStream().find(':');
        deserializeJson(doc, http.getStream());

        // Add to coin or portfolio
        if (app_mode == 1) {
          for (int i = 0; i < selected_coins_count; i++) {
            if (strcmp(id_buffer, selected_coins[i] -> coin_id) == 0){
              selected_coins[i] -> current_price = doc[currency_options_lower[selected_currency]];
              selected_coins[i] -> current_change = doc[currency_options_changes[selected_currency]];
              selected_coins[i] -> candles -> addPrice(doc[currency_options_lower[selected_currency]]);
              break;
            }
          }
        } else if (app_mode == 2) {
          int j = 0;
          while (portfolio_editor->selected_portfolio_indexes[j] != -1) {
            if (strcmp(id_buffer, coins[portfolio_editor->selected_portfolio_indexes[j]].coin_id) == 0){
              coins[portfolio_editor->selected_portfolio_indexes[j]].current_price =
                  doc[currency_options_lower[selected_currency]];
              coins[portfolio_editor->selected_portfolio_indexes[j]].current_change =
                  doc[currency_options_changes[selected_currency]];
              break;
            }
            j++;
          }
        }
    } while (http.getStream().findUntil(",\"", "}")); // Continue until last '}' reached

    Serial.println("Success");
   
    http.end();
  } else {
    http.end();
    
    // If WiFi becomes disconnected, load credentials and reconnect
    char loaded_ssid[31];
    char loaded_password[31];

    loadCredsFromEEPROM(loaded_ssid, loaded_password);

    WiFi.disconnect();
    WiFi.begin(loaded_ssid, loaded_password);

    while (WiFi.status() != WL_CONNECTED){
      delay(500);
    }
  }
}

int verifyID(char *id) {
  // Construct URL for passed coin id
  constructURL(id, request_url);

  // Init wifi client
  WiFiClientSecure client;
  client.setInsecure();

  StaticJsonDocument<90> doc;

  // Init HTTP
  HTTPClient http;
  http.useHTTP10(true);
  http.begin(client, request_url);
  
  int httpCode = http.GET();

  if (httpCode == 200) {
    // Get the request response payload (Price data)
    DeserializationError error = deserializeJson(doc, http.getStream());
    if (error) {
      http.end(); // Close connection
      return 0;
    }

    http.end();
    if (doc.size() != 1) {
      return 0; // Fail
    }
    return 1; // Success
  }
  
  http.end();
  return 0; // Fail
}

/**
 * Constructs the full URL for data retrieval.
 */
void constructURL(int app_mode, char* url_buffer) {
  int curr_url_pos = 0;
  int i = 0;

  // Add start of url
  while (url_start[i] != 0) {
    url_buffer[curr_url_pos] = url_start[i];
    curr_url_pos++;
    i++;
  }

  // Add coins selected in the specific coin part
  if (app_mode == 1) {
    // Add all coins to url except last
    for (int j = 0; j < selected_coins_count - 1; j++) {
      i = 0;
      while (selected_coins[j]->coin_id[i] != 0) {
        url_buffer[curr_url_pos] = selected_coins[j]->coin_id[i];
        curr_url_pos++;
        i++;
      }

      url_buffer[curr_url_pos] = '%';
      url_buffer[curr_url_pos + 1] = '2';
      url_buffer[curr_url_pos + 2] = 'C';
      curr_url_pos += 3;
    }

    // Add final coin to url
    i = 0;
    while (selected_coins[selected_coins_count - 1]->coin_id[i] != 0) {
      url_buffer[curr_url_pos] = selected_coins[selected_coins_count - 1]->coin_id[i];
      curr_url_pos++;
      i++;
    }
  } else if (app_mode == 2) {
    // Add all coins to url except last
    int j = 0;
    while (portfolio_editor->selected_portfolio_indexes[j] != -1) {
      i = 0;
      while (
          coins[portfolio_editor->selected_portfolio_indexes[j]].coin_id[i] !=
          0) {
        url_buffer[curr_url_pos] =
            coins[portfolio_editor->selected_portfolio_indexes[j]].coin_id[i];
        curr_url_pos++;
        i++;
      }

      url_buffer[curr_url_pos] = '%';
      url_buffer[curr_url_pos + 1] = '2';
      url_buffer[curr_url_pos + 2] = 'C';
      curr_url_pos += 3;

      j++;
    }
  }

  // Add ending to URL
  i = 0;
  while (url_end[i] != 0) {
    url_buffer[curr_url_pos] = url_end[i];
    curr_url_pos++;
    i++;
  }
    
  for (int j = 0; j < 3; j++){
    url_buffer[curr_url_pos] = currency_options
      [coin_menu->getButtons()[3].selectors[2].getSelected()[0]][j];
    curr_url_pos++;
  }

  url_buffer[curr_url_pos] = 0;
}

/**
 * Constructs URL for single coin ID
 */
void constructURL(char *coin_id, char* url_buffer) {
  int curr_url_pos = 0;
  int i = 0;

  // Add start of url
  while (url_start[i] != 0) {
    url_buffer[curr_url_pos] = url_start[i];
    curr_url_pos++;
    i++;
  }

  // Add coins selected in the specific coin part

  // Add final coin to url
  i = 0;
  while (coin_id[i] != 0) {
    url_buffer[curr_url_pos] = coin_id[i];
    curr_url_pos++;
    i++;
  }

  // Add ending to URL
  i = 0;
  while (url_end[i] != 0) {
    url_buffer[curr_url_pos] = url_end[i];
    curr_url_pos++;
    i++;
  }
  
  for (int j = 0; j < 3; j++){
    url_buffer[curr_url_pos] = currency_options
      [coin_menu->getButtons()[3].selectors[2].getSelected()[0]][j];
    curr_url_pos++;
  }

  url_buffer[curr_url_pos] = 0;
}

/**
 * Initialises network and displays initialisation process + details.
 */
void initialiseNetwork(char *ssid, char *password) {
  tft.fillScreen(BLACK);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(0, 10);
  tft.println(" SSID: ");
  tft.setTextColor(LIGHT_RED);
  tft.print(" ");
  tft.println(ssid);
  tft.setTextColor(WHITE);
  tft.println("\n Password: ");
  tft.setTextColor(LIGHT_RED);
  tft.print(" ");
  tft.println(password);
  tft.setTextColor(WHITE);
  WiFi.begin(ssid, password);
  tft.print("\n ");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    i++;
    if (i < 20){
      tft.fillRect(5 + ((i-1) * (tft.width()-10)/30), 58, (tft.width()-10)/30, 8, WHITE);
    } else {
      tft.fillRect(5 + ((i-1) * (tft.width()-10)/30), 58, (tft.width()-10)/30, 8, RED);
    }

    if (i == 30){
      ESP.restart();
    }
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  tft.print("\n ");
  tft.setTextColor(LIGHT_GREEN);
  tft.println("\n Connection Established");
  tft.setTextColor(WHITE);
  tft.println("\n IP address: ");
  tft.print(" ");
  tft.println(WiFi.localIP());
  delay(5000);
  tft.fillScreen(BLACK);
}


/**
 * Resets all of the coins (clears candles).
 */
void resetCoins() {
  // Free current candles
  for (int i = 0; i < selected_coins_count; i++) {
    selected_coins[i]->freeCandles();
  }
  
  selected_coins_count = 0;
  current_coin = 0;

  // Get all selected coins
  for (int i = 0; i < MAX_SELECTED_COINS; i++) {
    if (coin_menu->getButtons()[0].selectors[0].getSelected()[i] == -1)
      break;
      
    selected_coins_count++;
    selected_coins[i] =
        coins + coin_menu->getButtons()[0].selectors[0].getSelected()[i];

    selected_coins[i]->initCandles(&tft);
  }

  getData(1);
}


/**
 * Updates and draws the current time.
 */
void updateTime() {
  // Check minutes and update time
  long ms = millis();

  // Update current second
  current_second = int((ms/1000) - (60 * second_offset_counter));
  if (current_second >= 60){
    second_offset_counter++;
    current_second -= 60;
  }

  // Update current minute
  current_minute = int(((ms / (1000 * 60)) % 60) - (60 * minute_offset_counter));
  if (current_minute >= 60){
    minute_offset_counter++;
    current_minute -= 60;
  }
}

/**
 * Draws a passed bitmap on the screen at the given position with the given
 * colour.
 */
void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                int16_t h, uint16_t color) {
  int16_t i, j, byteWidth = (w + 7) / 8;
  uint8_t byte;
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      if (i & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(bitmap + j * byteWidth + i / 8);
      if (byte & 0x80)
        tft.drawPixel(x + i, y + j, color);
    }
  }
}

void writeCoinToEEPROM(COIN* coin, char index){
  // Increment the added coin count
  byte coin_count = EEPROM.read(COIN_COUNT_ADDRESS);
  
  // Locate where to write/replace
  int write_address = COIN_START_ADDRESS; // Go over the filled indicator
  int index_match = 0;

  // Increment write address, checking that index is not already allocated
  // (so that it can be replaced)
  for (byte i = 0; i < (int) coin_count; i++){
    byte ind = EEPROM.read(write_address + 1);
    
    if(byte(index) == ind){
      index_match = 1;
      break;
    }

    write_address += COIN_BLOCK_SIZE;
  }

  if (index_match == 0 && coin_count < ADDED_COINS_LIMIT){
    EEPROM.write(COIN_COUNT_ADDRESS, coin_count + 1);
  }

  if (index_match == 0 && coin_count >= ADDED_COINS_LIMIT){
    write_address = COIN_START_ADDRESS;
  }

  // Indicate that block is now filled 
  EEPROM.write(write_address, 1);
  write_address++;

  // Write index char to memory
  EEPROM.write(write_address, index);
  write_address++;
  
  // Write name to eeprom
  int i = 0;
  do {
    EEPROM.write(write_address+i, coin->coin_code[i]);
    i++;
  } while (coin->coin_code[i] != 0 && i < 8);
  EEPROM.write(write_address+i, 0);
  write_address+=9;

  // Write tag to eeprom
  i = 0;
  do {
    EEPROM.write(write_address+i, coin->coin_id[i]);
    i++;
  } while (coin->coin_id[i] != 0 && i < 30);
  EEPROM.write(write_address+i, 0);
  write_address+=31;

  // Write the colour  
  EEPROM.write(write_address, (byte) coin_changer->pickers[0].getValue());
  EEPROM.write(write_address+1, (byte) coin_changer->pickers[1].getValue());
  EEPROM.write(write_address+2, (byte) coin_changer->pickers[2].getValue());

  EEPROM.commit();
}

void readCoinsFromEEPROM(){
  // Get the current stored coin count so that it is known how many coins to load
  byte count = EEPROM.read(COIN_COUNT_ADDRESS);

  int write_address = COIN_START_ADDRESS;
  for (byte i = 0; i < count; i++){
    if (EEPROM.read(write_address) == 1){
      write_address++;
      byte index = EEPROM.read(write_address);
      write_address++;

      // Read coin code
      for (int i = 0; i < 9; i++){
        coins[index].coin_code[i] = (char) EEPROM.read(write_address+i);
      }
      write_address+=9;

      // Read coin id
      for (int i = 0; i < 31; i++){
        coins[index].coin_id[i] = EEPROM.read(write_address+i);
      }
      write_address+=31;

      coins[index].bitmap_present = 0;

      // Read coin colour
      coins[index].portfolio_colour = coin_changer->rgb_to_bgr(
        int(EEPROM.read(write_address)), int(EEPROM.read(write_address+1)), int(EEPROM.read(write_address+2)));
      write_address+=3;

      coin_list[index] = coins[index].coin_code;
    }
  }
}

void savePortfolioToEEPROM(){
    // Write amount owned to eeprom
    char double_buffer[18];
    int block_address = PORTFOLIO_START_ADDRESS;
    int write_address = block_address;
    byte j = 0;
    
    for (int i = 0; i < COIN_COUNT; i++){
      if (coins[i].amount > 0 && j < PORTFOLIO_LIMIT){
        write_address = block_address;
        j++;
        for (int k = 0; k < 18; k++){
          double_buffer[k] = 0;
        }

        // Index the amount is associated with
        EEPROM.write(write_address, byte(i));
        write_address++;

        // Load double amount into string buffer
        dtostrf(coins[i].amount, 10, 6, double_buffer);

        // Write amount to block
        for (int k = 0; k < 18; k++){
          EEPROM.write(write_address, double_buffer[k]);
          write_address++;
        }

        // Move to next block
        block_address+=PORTFOLIO_BLOCK_SIZE;
      }
    }

    EEPROM.write(PORTFOLIO_COUNT_ADDRESS, j);
}

void loadPortfolioFromEEPROM(){
  byte count = EEPROM.read(PORTFOLIO_COUNT_ADDRESS);
  int block_address = PORTFOLIO_START_ADDRESS;
  int write_address;
  char dub[18];

  // Load portfolio from EEPROM
  for (byte i = 0; i < count; i++){
    write_address = block_address;
    int index = EEPROM.read(write_address);

    write_address++;
    for (int i = 0; i < 18; i++){
      dub[i] = EEPROM.read(write_address+i);
    }
    
    coins[index].amount = strtod(dub, NULL);

    block_address+=PORTFOLIO_BLOCK_SIZE;
  }

  // Refresh portfolio to display coins with non-zero amounts
  if (count > 0){
    portfolio->refreshSelectedCoins();
  }
}

void writeSettingsToEEPROM(){
  int write_address = SETTINGS_START_ADDRESS;
  
  // Create the string that specifies selected coins
  for (int i = 0; i < MAX_SELECTED_COINS; i++) {
    if(coin_menu->getButtons()[0].selectors[0].getSelected()[i] == -1){
      EEPROM.write(write_address, (byte) 255);
    } else {
      EEPROM.write(write_address, (byte) coin_menu->getButtons()[0].selectors[0].getSelected()[i]);
    }
    write_address++;
  }
  
  // Crypto display settings
  for (int i = 0; i < 4; i++){
    EEPROM.write(write_address, (byte) coin_menu->getButtons()[3].selectors[i].getSelected()[0]);
    write_address++;
  }

  // Portfolio setting
  for (int i = 0; i < 2; i++){
    EEPROM.write(write_address, (byte) coin_menu->getButtons()[4].selectors[i].getSelected()[0]);
    write_address++;
  }
}

void loadSettingsFromEEPROM(){
  // Read the settings string and modify arrays accordingly
  int read_address = SETTINGS_START_ADDRESS;
  
  // Create the string that specifies selected coins
  for (int i = 0; i < MAX_SELECTED_COINS; i++) {
    if (EEPROM.read(read_address) == 255){
      read_address++;
    } else {
      coin_menu -> getButtons()[0].selectors[0].setSelected(i, (int) EEPROM.read(read_address));
      selected_coins[i] =
        coins + (int) EEPROM.read(read_address);

      selected_coins[i] -> initCandles(&tft);
      
      selected_coins_count++;
      read_address++;
    }
  }
  
  // Crypto display settings
  for (int i = 0; i < 4; i++){
    coin_menu -> getButtons()[3].selectors[i].setSelected(0, (int) EEPROM.read(read_address));
    read_address++;
  }

  // Portfolio setting
  for (int i = 0; i < 2; i++){
    coin_menu -> getButtons()[4].selectors[i].setSelected(0, (int) EEPROM.read(read_address));
    read_address++;
  }
}


/**
 * Clears the EEPROM memory.
 */
void clearEEPROM() {
  for (int i = 0; i < EEPROM_SIZE; i++)
    EEPROM.write(i, 0);

  for (int i = SETTINGS_START_ADDRESS; i < SETTINGS_START_ADDRESS + MAX_SELECTED_COINS; i++){
    EEPROM.write(i, 255);
  }

  EEPROM.commit();
}

void clearEEPROMArea(int start_addr, int end_addr){
  for (int i = start_addr; i < end_addr; i++)
    EEPROM.write(i, 0);

  EEPROM.commit();
}

/**
 * Writes a string to the EEPROM.
 */
void writeToEEPROM(char *input) {
  int i = 0;
  do {
    EEPROM.write(eeprom_address, input[i]);
    eeprom_address++;
    i++;
  } while (input[i] != 0);

  EEPROM.write(eeprom_address, '\0');
  eeprom_address++;
}

void printEEPROM(){
 Serial.print("EEPROM State:");
  
  for (int i = 0; i < 512; i++){
      if (i%32 == 0){
       Serial.println();
      }
    
      byte read_value = EEPROM.read(i);

      if (read_value < 10){
       Serial.print("  ");
      } else if (read_value < 100){
       Serial.print(' ');
      }
      
     Serial.print(read_value);

     Serial.print(' ');
  }
 Serial.println();
}

/**
 * Loads the credits from the EEPROM, the format is {0/1}*NAME*\0*PASSWORD*.
 */
void loadCredsFromEEPROM(char *ssid, char *pass) {
  int addr = 1;

  // Read SSID
  byte read_from_eeprom = EEPROM.read(addr);
  int i = 0;
  while (read_from_eeprom != 0) {
    ssid[i] = (char)read_from_eeprom;
    addr++;
    i++;
    read_from_eeprom = EEPROM.read(addr);
  }
  ssid[i] = 0;
  addr++;

  // Read Password
  read_from_eeprom = EEPROM.read(addr);
  i = 0;
  while (read_from_eeprom != 0) {
    pass[i] = (char)read_from_eeprom;
    addr++;
    i++;
    read_from_eeprom = EEPROM.read(addr);
  }
  pass[i] = 0;
}

/**
 * Interacts with the menu, catches keys pressed and performs corresponding
 * menu actions.
 */
void interactWithMenu() {
  if (irrecv.decode()) {     
      // Down key pressed, select next element in menu
      if (irrecv.decodedIRData.decodedRawData == IR_DOWN)
        coin_menu->moveDown();

      // Down key pressed, select previous element in menu
      if (irrecv.decodedIRData.decodedRawData == IR_UP)
        coin_menu->moveUp();

      // # to exit menu
      if (irrecv.decodedIRData.decodedRawData == IR_HASHTAG){
        in_menu = 0;

        tft.fillScreen(ST77XX_BLACK);
        drawFetchingData();
        
        getData(1); 

        savePortfolioToEEPROM();
        writeSettingsToEEPROM();
        EEPROM.commit();
        
        portfolio->refreshSelectedCoins(); // Add any new coins to selected
        
        // Get data if portfolio not empty
        if (portfolio_editor->selected_portfolio_indexes[0] != -1){
          getData(2); // Get data for portfolio
        }
        
        portfolio->addPriceToCandles();
        
        if (mode == 1) {
          selected_coins[current_coin]->display(&tft, coin_menu->getButtons()[3].selectors[2].getSelected()[0]);

          incrementCoinCycleDelay();
        }
        
        if (mode == 2) portfolio->display(coin_menu->getButtons()[3].selectors[2].getSelected()[0]);
        
        if (mode == 3) displayAllCoins();
      }

      // OK pressed, see what button was pressed and take action
      if (irrecv.decodedIRData.decodedRawData == IR_OK) {
        char *button_action = coin_menu->press();

        if (button_action == "Select Coins") {
          in_coinlist = 1;
          coin_menu->getButtons()[0].drawSubMenu();
        }

        if (button_action == "Add New Coin") {
          coin_changer->setActive();
          coin_changer->display();
        }

        if (button_action == "Crypto Display Settings") {
          in_crypto_settings = 1;
          coin_menu->getButtons()[3].drawSubMenu();
        }

        if (button_action == "Clear WiFi Credentials")
          clearEEPROMArea(0, SETTINGS_START_ADDRESS);

        if (button_action == "Reset Coins (Restart)"){
          clearEEPROMArea(COIN_COUNT_ADDRESS, PORTFOLIO_COUNT_ADDRESS);
          ESP.restart();
        }

        if (button_action == "Portfolio Settings") {
          in_portfolio_settings = 1;
          coin_menu->getButtons()[4].drawSubMenu();
        }

        if (button_action == "Edit Portfolio") {
          portfolio_editor->setActive();
          portfolio_editor->display();
        }

        if (button_action == "Clear Portfolio"){
          clearEEPROMArea(PORTFOLIO_COUNT_ADDRESS, EEPROM_SIZE);

          for (int i = 0; i < COIN_COUNT; i++)
            coins[i].amount = 0;
        }
      }

      delay(50);
      irrecv.resume();
    }
  }

void drawFetchingData(){
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(30, tft.height()/2);
  tft.print("Fetching data...");
}


void drawMenu() {
  coin_menu->display();

  // Display connection status messages
  tft.setTextSize(1);
  if (WiFi.status() == WL_CONNECTED && coingecko_up == 1){
    tft.setCursor(52, tft.height()-10);
    tft.setTextColor(LIGHT_GREEN);
    tft.print("Connected");
  } else {
    tft.setTextColor(LIGHT_RED);
    if (WiFi.status() != WL_CONNECTED){
      tft.setCursor(29, tft.height()-10);
      tft.print("WiFi Disconnected");
    } else if (coingecko_up == 0){
      tft.setCursor(8, tft.height()-10);
      tft.print("CoinGecko Not Responding");
    }
  }
}

/**
 * Interacts with the coin list submenu, catches keys pressed and performs
 * corresponding menu actions.
 */
void interactWithCoinList() {
  coin_menu->getButtons()[0].flashSelectedSelector();
  if (irrecv.decode()) {
    // Exit sub menu
    if (irrecv.decodedIRData.decodedRawData == IR_HASHTAG) {
      tft.fillScreen(BLACK);
      drawFetchingData();
      resetCoins();
      in_coinlist = 0;
      drawMenu();
    }
    
    if (irrecv.decodedIRData.decodedRawData == IR_OK) {
      char *action = coin_menu->getButtons()[0].pressSubMenu();
    }

    if (irrecv.decodedIRData.decodedRawData == IR_DOWN)
      coin_menu->getButtons()[0].subMenuDown();

    if (irrecv.decodedIRData.decodedRawData == IR_UP)
      coin_menu->getButtons()[0].subMenuUp();

    if (irrecv.decodedIRData.decodedRawData == IR_LEFT)
      coin_menu->getButtons()[0].subMenuLeft();

    if (irrecv.decodedIRData.decodedRawData == IR_RIGHT)
      coin_menu->getButtons()[0].subMenuRight();

    delay(50);
    irrecv.resume();
  }
}

/**
 * Interacts with the settings sub menu, catches keys pressed and performs
 * corresponding menu actions.
 */
void interactWithCryptoSettings() {
    coin_menu->getButtons()[3].flashSelectedSelector();
    if (irrecv.decode()) {
      // Exit sub menu
      if (irrecv.decodedIRData.decodedRawData == IR_HASHTAG) {
        // Update selected settings
        coin_cycle_delay = coin_change_times_values[coin_menu->getButtons()[3].selectors[0].getSelected()[0]];
        coin_candle_update_delay = candle_change_times_values[coin_menu->getButtons()[3].selectors[1].getSelected()[0]];

        if (coin_candle_update_delay != last_coin_candle_update_delay || 
              last_currency != coin_menu -> getButtons()[3].selectors[2].getSelected()[0]){
          tft.fillScreen(ST77XX_BLACK);
          drawFetchingData();
          resetCoins();
        }

        last_coin_candle_update_delay = coin_candle_update_delay;
        
        // If currency changes, also reset portfolio candles
        if (last_currency != coin_menu -> getButtons()[3].selectors[2].getSelected()[0]){
          // Reset portfolio candles and data
          portfolio->clearCandles();
          
          // Get data if portfolio not empty
          if (portfolio_editor->selected_portfolio_indexes[0] != -1)
            getData(2); // Get data for portfolio
          
          portfolio->addPriceToCandles();
          
          last_currency = coin_menu -> getButtons()[3].selectors[2].getSelected()[0];
        }

        if (last_bitmap_toggle != coin_menu -> getButtons()[3].selectors[3].getSelected()[0]){
          for (int i = 0; i < COIN_COUNT; i++)
            coins[i].toggleBitmap();

          last_bitmap_toggle = coin_menu -> getButtons()[3].selectors[3].getSelected()[0];
        }

        in_crypto_settings = 0;
        drawMenu();
      }

      // Pass pressed buttons into coin settings sub menu
      if (irrecv.decodedIRData.decodedRawData == IR_OK) {
        char *action = coin_menu->getButtons()[3].pressSubMenu();
      }

      if (irrecv.decodedIRData.decodedRawData == IR_DOWN)
        coin_menu->getButtons()[3].subMenuDown();

      if (irrecv.decodedIRData.decodedRawData == IR_UP)
        coin_menu->getButtons()[3].subMenuUp();

      if (irrecv.decodedIRData.decodedRawData == IR_LEFT)
        coin_menu->getButtons()[3].subMenuLeft();

      if (irrecv.decodedIRData.decodedRawData == IR_RIGHT)
        coin_menu->getButtons()[3].subMenuRight();

      delay(50);
      irrecv.resume();
    }
}

void interactWithPortfolioSettings() {
    coin_menu->getButtons()[4].flashSelectedSelector();
    if (irrecv.decode()) {
      // Exit sub menu
      if (irrecv.decodedIRData.decodedRawData == IR_HASHTAG) {
        portfolio_candle_update_delay = candle_change_times_values
            [coin_menu->getButtons()[4].selectors[0].getSelected()[0]];

        if (portfolio_candle_update_delay !=
            last_portfolio_candle_update_delay)
          portfolio->clearCandles();

        last_portfolio_candle_update_delay = portfolio_candle_update_delay;

        portfolio_cycle_disabled = coin_menu->getButtons()[4].selectors[1].getSelected()[0];

        in_portfolio_settings = 0;
        drawMenu();
      }

      // Pass pressed buttons into portfolio settings sub menu
      if (irrecv.decodedIRData.decodedRawData == IR_OK) 
        char *action = coin_menu->getButtons()[4].pressSubMenu();

      if (irrecv.decodedIRData.decodedRawData == IR_DOWN)
        coin_menu->getButtons()[4].subMenuDown();

      if (irrecv.decodedIRData.decodedRawData == IR_UP)
        coin_menu->getButtons()[4].subMenuUp();

      if (irrecv.decodedIRData.decodedRawData == IR_LEFT)
        coin_menu->getButtons()[4].subMenuLeft();

      if (irrecv.decodedIRData.decodedRawData == IR_RIGHT)
        coin_menu->getButtons()[4].subMenuRight();

      delay(50);
      irrecv.resume();
    }
}
