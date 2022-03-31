#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <ArduinoJson.h>
#include <Colours.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <IRremote.h>
#include <NTPClient.h>
#include <SPI.h>
#include <ST7735_Candle_Graph.h>
#include <ST7735_Coin.h>
#include <ST7735_Coin_Changer.h>
#include <ST7735_Keyboard.h>
#include <ST7735_Menu.h>
#include <ST7735_Portfolio.h>
#include <ST7735_Portfolio_Editor.h>
#include <WiFiUdp.h>

extern unsigned char epd_bitmap_name[];
extern unsigned char epd_bitmap_logo[];
extern unsigned char epd_bitmap_logo_cropped[];

// Coin bitmaps
extern unsigned char epd_bitmap_bitcoin[];
extern unsigned char epd_bitmap_ethereum[];
extern unsigned char epd_bitmap_tether[];
extern unsigned char epd_bitmap_cardano[];
extern unsigned char epd_bitmap_binance[];
extern unsigned char epd_bitmap_xrp[];
extern unsigned char epd_bitmap_solana[];
extern unsigned char epd_bitmap_dot[];
extern unsigned char epd_bitmap_dogecoin[];
extern unsigned char epd_bitmap_terra[];
extern unsigned char epd_bitmap_litecoin[];
extern unsigned char epd_bitmap_avax[];
extern unsigned char epd_bitmap_algorand[];
extern unsigned char epd_bitmap_link[];
extern unsigned char epd_bitmap_matic[];
extern unsigned char epd_bitmap_tron[];
extern unsigned char epd_bitmap_tether[];
extern unsigned char epd_bitmap_bat[];
extern unsigned char epd_bitmap_uniswap[];

// Define PIN config
#define TFT_CS D3
#define TFT_RST D4
#define TFT_DC D2
#define TFT_SCLK D5
#define TFT_MOSI D7
#define RECV_PIN D1

#define MAX_SELECTED_COINS 6

#define COIN_MENU_BUTTON_COUNT 5
#define PORTFOLIO_MENU_BUTTON_COUNT 4

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft =
    Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// For interface
int selected_coins_count = 1;   // Current number of coins selected
int current_coin = -1;               // Index of current coin being displayed
int current_second;             // Current second from time NTP server
unsigned long last_minute = -1; // Last minute for time update check

// Flags
int ssid_entered;            // Indicates if SSID of network entered
int password_entered;        // Indicates if password of network entered
int network_initialised = 0; // Indicates if network has been initialised
int eeprom_address = 0;      // Current address being written to on EEPROM
int in_menu = 0;             // Indicates if currently in the menu
int in_settings = 0;         // Indicates if currently in settings submenu
int in_coinlist = 0;         // Indicates if currently in coin list submenu
int in_portfolio_editor = 0;
int coin_time_slot_moved = 0; // Indicates if coin candles have been moved along
int portfolio_time_slot_moved =
    0; // Indicates if portfolio candles have been moved along

// For HTTP connection
WiFiClientSecure client;
HTTPClient http;
const char *fingerprint = "33c57b69e63b765c393df1193b1768b81b0a1fd9";
char *url_start = "https://api.coingecko.com/api/v3/simple/price?ids=";
char *url_end = "&vs_currencies=gbp&include_24hr_change=true";

// For time retrieval
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                             "Thursday", "Friday", "Saturday"};
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// IR remote
IRrecv irrecv(RECV_PIN);
decode_results results;

// Instantiate keyboard
ST7735_Keyboard *keyboard;

// Instantiate menu
ST7735_Menu *coin_menu;
ST7735_Menu *portfolio_menu;
char **coin_menu_button_functions;
char **portfolio_menu_button_functions;
char *coin_change_times[5] = {"5", "10", "15", "30", "60"};

// Instantiate components of Portfolio
ST7735_Portfolio_Editor *portfolio_editor;
ST7735_Portfolio *portfolio;
ST7735_Coin_Changer *coin_changer;

char **coin_list;             // List of all coin codes
int coin_cycle_delay;         // Delay between coin changes
int next_coin_change = -1;    // Allows temporary extension of coin change delay
int coin_candle_update_delay; // How often candles are pushed along (mins)
int last_coin_candle_update_delay =
    5;                             // To detect changes in candle update time
int portfolio_candle_update_delay; // How often candles are pushed along (mins)
int last_portfolio_candle_update_delay =
    5; // To detect changes in candle update time

int mode = 1; // Coin or portfolio mode

COIN *coins;           // List of coins that can be selected
COIN **selected_coins; // List of pointers to coins currently selected

void setup(void) {
  irrecv.enableIRIn(); // Enable IR reciever

  // Initialise display
  tft.initR(INITR_GREENTAB); // Init ST7735S chip
  //tft.initR(INITR_BLACKTAB); // Init ST7735S chip
  tft.setRotation(1);

  // Initialise keyboard
  keyboard = (ST7735_Keyboard *)malloc(sizeof(ST7735_Keyboard));
  *keyboard = ST7735_Keyboard(&tft);

  // Initialise coin menu
  coin_menu_button_functions =
      (char **)malloc(sizeof(char *) * COIN_MENU_BUTTON_COUNT);

  coin_menu_button_functions[0] = "Edit Coin List";
  coin_menu_button_functions[1] = "Add New Coin";
  coin_menu_button_functions[2] = "Crypto Display Settings";
  coin_menu_button_functions[3] = "Clear WiFi Credentials";
  coin_menu_button_functions[4] = "Exit Menu";
  coin_menu = (ST7735_Menu *)malloc(sizeof(ST7735_Menu));
  *coin_menu =
      ST7735_Menu(&tft, COIN_MENU_BUTTON_COUNT, coin_menu_button_functions);

  // Initialise portfolio menu
  portfolio_menu_button_functions =
      (char **)malloc(sizeof(char *) * PORTFOLIO_MENU_BUTTON_COUNT);
  portfolio_menu_button_functions[0] = "Edit Portfolio";
  portfolio_menu_button_functions[1] = "Portfolio Settings";
  portfolio_menu_button_functions[2] = "Clear WiFi Credentials";
  portfolio_menu_button_functions[3] = "Exit Menu";
  portfolio_menu = (ST7735_Menu *)malloc(sizeof(ST7735_Menu));
  *portfolio_menu = ST7735_Menu(&tft, PORTFOLIO_MENU_BUTTON_COUNT,
                                portfolio_menu_button_functions);

  // Create coin array
  coins = (COIN *)malloc(sizeof(COIN) * COIN_COUNT);
  selected_coins = (COIN **)malloc(sizeof(COIN *) * MAX_SELECTED_COINS);

  coin_list = (char **)malloc(sizeof(char *) * COIN_COUNT);

  coins[0] = COIN("BTC", "bitcoin", epd_bitmap_bitcoin, GOLD, WHITE, GOLD, 0);
  coins[1] = COIN("ETH", "ethereum", epd_bitmap_ethereum, WHITE, GRAY, GRAY, 0);
  coins[2] =
      COIN("LTC", "litecoin", epd_bitmap_litecoin, GRAY, WHITE, DARK_GREY, 0);
  coins[3] =
      COIN("ADA", "cardano", epd_bitmap_cardano, LIGHT_BLUE, WHITE, LIGHT_BLUE, 0);
  coins[4] = COIN("BNB", "binancecoin", epd_bitmap_binance, WHITE, GOLD, GOLD, 0);
  coins[5] = COIN("USDT", "tether", epd_bitmap_tether, GREEN, WHITE, GREEN, 0);
  coins[6] = COIN("XRP", "ripple", epd_bitmap_xrp, DARK_GREY, WHITE, DARK_GREY, 0);
  coins[7] = COIN("DOGE", "dogecoin", epd_bitmap_dogecoin, GOLD, WHITE, GOLD, 0);
  coins[8] = COIN("DOT", "polkadot", epd_bitmap_dot, WHITE, BLACK, WHITE, 0);
  coins[9] =
      COIN("SOL", "solana", epd_bitmap_solana, PINK, LIGHTNING_BLUE, PINK, 0);
  coins[10] = COIN("LUNA", "terra-luna", epd_bitmap_terra, NIGHT_BLUE,
                   MOON_YELLOW, MOON_YELLOW, 0);
  coins[11] =
      COIN("ALGO", "algorand", epd_bitmap_algorand, WHITE, BLACK, WHITE, 0);
  coins[12] =
      COIN("LINK", "chainlink", epd_bitmap_link, DARK_BLUE, WHITE, DARK_BLUE, 0);
  coins[13] =
      COIN("MATIC", "matic-network", epd_bitmap_matic, PURPLE, WHITE, PURPLE, 0);
  coins[14] = COIN("TRX", "tron", epd_bitmap_tron, RED, WHITE, RED, 0);
  coins[15] = COIN("BAT", "basic-attention-token", epd_bitmap_bat, WHITE,
                   SALMON, SALMON, 0);
  coins[16] = COIN("AVAX", "avalanche-2", epd_bitmap_avax, RED, WHITE, RED, 0);
  coins[17] =
      COIN("UNI", "uniswap", epd_bitmap_uniswap, WHITE, HOT_PINK, HOT_PINK, 0);

  // Add selectors to respective submenus
  coin_menu->getButtons()[0].addSelector(
      "Coin duration (secs):", coin_change_times, 5, 1, 5);
  coin_menu->getButtons()[0].addSelector(
      "Candle duration (mins):", coin_change_times, 5, 1, 5);
  coin_menu->getButtons()[1].addSelector("Select up to 5 coins:", coin_list, 3,
                                         MAX_SELECTED_COINS, COIN_COUNT);

  portfolio_menu->getButtons()[0].addSelector(
      "Candle duration (mins):", coin_change_times, 5, 1, 5);

  for (int i = 0; i < COIN_COUNT; i++)
    coin_list[i] = coins[i].coin_code;

  // Initialise portfolio
  portfolio_editor =
      (ST7735_Portfolio_Editor *)malloc(sizeof(ST7735_Portfolio_Editor));
  *portfolio_editor = ST7735_Portfolio_Editor(&tft, coins);

  portfolio = (ST7735_Portfolio *)malloc(sizeof(ST7735_Portfolio));
  *portfolio = ST7735_Portfolio(&tft, portfolio_editor, coins);

  coin_changer = (ST7735_Coin_Changer *)malloc(sizeof(ST7735_Coin_Changer));
  *coin_changer = ST7735_Coin_Changer(&tft, coin_list, coins, keyboard,
                                      epd_bitmap_logo_cropped);

  // Get default coin delay time from selector
  String str = coin_change_times
      [coin_menu->getButtons()[0].selectors[0].getSelected()[0]];
  coin_cycle_delay = str.toInt();

  // Get default update time from selector
  str = coin_change_times
      [coin_menu->getButtons()[0].selectors[0].getSelected()[0]];
  coin_candle_update_delay = str.toInt();
  portfolio_candle_update_delay = str.toInt();

  // Try to read network credentials from EEPROM if they exist
  EEPROM.begin(512);
  byte value = EEPROM.read(0);

  // First ever boot has random memory, need to clear
  if (value != 1 && value != 0)
    clearEEPROM();

  // Check if credits have been written, take necessary action
  if (value == 0) {
    ssid_entered = 0;
    password_entered = 0;
    keyboard->displayInstructions();
    keyboard->displayPrompt("Enter Network Name (SSID):");
    keyboard->display();
  } else if (value == 1) {
    ssid_entered = 1;
    password_entered = 1;
  }
}

void loop() {
  if (ssid_entered == 0) { // Get network name
    if (keyboard->enterPressed() == 1) {
      // Indicate SSID and pwd loaded into EEPROM
      EEPROM.write(eeprom_address, 1);
      eeprom_address++;
      writeToEEPROM(keyboard->getCurrentInput());
      keyboard->reset();

      ssid_entered = 1;

      keyboard->displayPrompt("Enter Network Password:.");
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
    // If first value of eeprom is null terminator then take input from user
    char *loaded_ssid = (char *)malloc(sizeof(char) * 30);
    char *loaded_password = (char *)malloc(sizeof(char) * 30);

    drawIntroAnimation();

    loadCredsFromEEPROM(loaded_ssid, loaded_password);

    initialiseNetwork(loaded_ssid, loaded_password);

    // Initialise the candle for first coin (bitcoin)
    selected_coins[0] = coins;
    selected_coins[0]->initCandles(&tft);

    forceGetData(1); // Get data for coin

    // Begin ndp time client and draw time on screen
    timeClient.begin();
    drawTime();

    network_initialised = 1; // Indicate successful network init
  } else if (in_menu == 1) { // Do menu operations
    if (in_coinlist == 1) {  // Interact with coin list submenu
      interactWithCoinList();
      return;
    }

    if (in_settings == 1) { // Interact with settings submenu
      interactWithSettings();
      return;
    }

    if (coin_changer->isActive() == 1) { // Interact with coin changer
      if (irrecv.decode()) {
        int response =
            coin_changer->interact(&irrecv.decodedIRData.decodedRawData);
        if (response == 0) {
          drawMenu();
        } else if (response == 1) {
          if (verifyID(coin_changer->loaded_id) == 1) {
            coin_changer->verificationSuccess();
          } else {
            coin_changer->verificationFailed();
          }
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
    updateAndDrawTime();
    
    for (int i = 0; i < 10; i++) {
      delay(50);
      if (irrecv.decode()) {
        // Check if '#' pressed to open menu
        if (irrecv.decodedIRData.decodedRawData == 0xF20DFF00) {
          in_menu = 1;
          drawMenu();
          delay(250);
          irrecv.resume();
          return;
        }

        // Change mode to opposite if up or down pressed
        if (irrecv.decodedIRData.decodedRawData == 0xE718FF00 ||
            irrecv.decodedIRData.decodedRawData == 0xAD52FF00) {
          mode = mode == 1 ? 2 : 1;
          irrecv.resume();

          if (mode == 1) {
            displayNextCoin();
          } else {
            portfolio->display();
          }

          break;
        }

        // Check if user wants to move to next/previous coin
        if (mode == 1) {
          // Previous coin
          if (irrecv.decodedIRData.decodedRawData == 0xF708FF00) {
            displayPreviousCoin();
          }

          // Next coin
          if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00) {
            displayNextCoin();
          }
        } else if (mode == 2) {
          if (irrecv.decodedIRData.decodedRawData == 0xF708FF00) {
            portfolio->previousMode();
            portfolio->display();
          } else if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00) {
            portfolio->nextMode();
            portfolio->display();
          }
        }

        irrecv.resume();
      }
    }

    if (next_coin_change == -1){
      displayNextCoin();
    }

    // Move coin data along every 'coin_candle_update_delay' minutes
    if (timeClient.getMinutes() % coin_candle_update_delay == 0 &&
        coin_time_slot_moved == 0) {
      for (int i = 0; i < selected_coins_count; i++)
        selected_coins[i]->candles->nextTimePeriod(
            selected_coins[i]->current_price);

      coin_time_slot_moved = 1;
    } else if (timeClient.getMinutes() % 5 != 0 && coin_time_slot_moved == 1) {
      coin_time_slot_moved = 0;
    }

    // Move portfolio data along every 'portfolio_candle_update_delay' minutes
    if (timeClient.getMinutes() % portfolio_candle_update_delay == 0 &&
        portfolio_time_slot_moved == 0) {
      portfolio->nextTimePeriod();

      portfolio_time_slot_moved = 1;
    } else if (timeClient.getMinutes() % 5 != 0 &&
               portfolio_time_slot_moved == 1) {
      portfolio_time_slot_moved = 0;
    }

    // Update coin and portfolio data every 60 seconds
    if (current_second == 0) {
      // Get individual coin data
      forceGetData(1);

      // Get data for portfolio coins
      portfolio->refreshSelectedCoins(); // Add any new coins to selected
      forceGetData(2);                   // Get data
      portfolio->addPriceToCandles();

      if (mode == 2)
        portfolio->display();
    }

    // Move to next coin every *selected* seconds if in crypto mode
    if (mode == 1) {
      if (current_second == next_coin_change) {
        displayNextCoin();
      }
    }
  }
}

void displayNextCoin(){      
  current_coin++;
  if (current_coin == selected_coins_count)
    current_coin = 0;

  selected_coins[current_coin]->display(&tft);

  incrementCoinCycleDelay();
}

void displayPreviousCoin(){
  if (current_coin == 0)
    current_coin = selected_coins_count;

  current_coin--;

  selected_coins[current_coin]->display(&tft);

  incrementCoinCycleDelay();
}

void incrementCoinCycleDelay(){
  next_coin_change = current_second + coin_cycle_delay;
  if (next_coin_change >= 60){
    next_coin_change -= 60;
  }
}

void drawIntroAnimation() {
  // Give user a chance to wipe stored WiFi details (e.g. if network changes)

  tft.fillScreen(BLACK);
  tft.setCursor(10, 5);
  drawBitmap(5, 20, epd_bitmap_logo, 45, 45, RED);
  drawBitmap(55, 15, epd_bitmap_name, 100, 58, WHITE);

  tft.setCursor(10, tft.height() - 30);
  tft.write("Press * to clear EEPROM");

  for (int i = 5; i <= tft.width() - 10; i += 3) {
    delay(50);
    tft.fillRect(i, tft.height() - 15, 3, 10, WHITE);

    if (checkForCredsClear() == 1) {
      password_entered = 0;
      ssid_entered = 0;
      return;
    }
  }
}

// Data Retrieval
/**
 * Returns the current time formatted as MM:SS
 */
void getFormattedTimeNoSeconds(NTPClient timeClient) {
  unsigned long rawTime = timeClient.getEpochTime();
//  unsigned long hours = (rawTime % 86400L) / 3600;
//  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
//
  unsigned long minutes = (rawTime % 3600) / 60;
  last_minute = minutes;
//  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);
//
//  return hoursStr + ":" + minuteStr;
}

/**
 * Attempts to get data from the coingecko api until it successfully gets it.
 */
void forceGetData(int app_mode) {
  while (getData(app_mode) == 0) {
    //Serial.println("Failed to get data, retrying...");
    delay(5000);
  }
}

/**
 * Retrieves the current price, and 24hr change of the currently selected coins,
 * uses CoinGecko API.
 */
int getData(int app_mode) {
  // Instanciate Secure HTTP communication
  client.setFingerprint(fingerprint);

  char url[200];
  constructURL(app_mode, url); // Get the URL

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    StaticJsonDocument<800> doc;
    // Get the request response payload (Price data)
    DeserializationError error = deserializeJson(doc, http.getString());
    if (error) {
      http.end(); // Close connection
      return 0;
    }

    JsonObject current;

    if (app_mode == 1) {
      for (int i = 0; i < selected_coins_count; i++) {
        current = doc[selected_coins[i]->coin_id];
        selected_coins[i]->current_price = current["gbp"];
        selected_coins[i]->current_change = current["gbp_24h_change"];

        // selected_coins[i] -> candles ->
        // addPrice(incrementForGraph(current["gbp"]));
        selected_coins[i]->candles->addPrice(current["gbp"]);
      }
    } else if (app_mode == 2) {
      int j = 0;
      while (portfolio_editor->selected_portfolio_indexes[j] != -1) {
        current =
            doc[coins[portfolio_editor->selected_portfolio_indexes[j]].coin_id];
        coins[portfolio_editor->selected_portfolio_indexes[j]].current_price =
            current["gbp"];
        coins[portfolio_editor->selected_portfolio_indexes[j]].current_change =
            current["gbp_24h_change"];
        j++;
      }
    }
  }
  http.end(); // Close connection
  return 1;
}

int verifyID(char *id) {
  // Instanciate Secure HTTP communication
  client.setFingerprint(fingerprint);

  char url[200];
  constructURL(id, url); // Get the URL

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    StaticJsonDocument<800> doc;
    // Get the request response payload (Price data)
    DeserializationError error = deserializeJson(doc, http.getString());
    if (error) {
      http.end(); // Close connection
      return 0;
    }

    http.end();
    if (doc.size() != 1) {
      return 0;
    }
    return 1;
  }
}

/**
 * Constructs the URL for data retrieval.
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
  url_buffer[curr_url_pos] = 0;
}

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
  tft.setTextColor(RED);
  tft.println(" " + String(ssid));
  tft.setTextColor(WHITE);
  tft.println("\n Password: ");
  tft.setTextColor(RED);
  tft.println(" " + String(password));
  tft.setTextColor(WHITE);
  WiFi.begin(ssid, password);
  tft.print("\n ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    tft.print(".");
  }

  tft.print("\n ");
  tft.setTextColor(ST77XX_GREEN);
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
  current_coin = -1;

  // Get all selected coins
  for (int i = 0; i < MAX_SELECTED_COINS; i++) {
    if (coin_menu->getButtons()[1].selectors[0].getSelected()[i] == -1)
      break;
      
    selected_coins_count++;
    selected_coins[i] =
        coins + coin_menu->getButtons()[1].selectors[0].getSelected()[i];

    selected_coins[i]->initCandles(&tft);
  }

  forceGetData(1);
}


/**
 * Draws the current time and day on the display.
 */
void drawTime() {
  tft.fillRect(0, tft.height() - 12, tft.width(), 12, BLACK);
  tft.setTextColor(WHITE);
  tft.setCursor(3, tft.height() - 10);
  tft.setTextSize(1);

  tft.print(daysOfTheWeek[timeClient.getDay()]);
  tft.setCursor(tft.width() - 32, tft.height() - 10);
  //tft.print(getFormattedTimeNoSeconds(timeClient));
  getFormattedTimeNoSeconds(timeClient);
}

/**
 * Updates and draws the current time.
 */
void updateAndDrawTime() {
  // Check minutes and update time
  while (!timeClient.update()) {
    delay(500);
  }

  current_second = timeClient.getSeconds();

  // Draw time if a minute has passed since last drawn
  if (last_minute != timeClient.getMinutes()) {
    drawTime();
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


/**
 * Clears the EEPROM memory.
 */
void clearEEPROM() {
  for (int i = 0; i < 512; i++)
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
 * Check if '*' pressed, if so, clears existing WiFi credentials.
 */
int checkForCredsClear() {
  if (irrecv.decode()) {
    if (irrecv.decodedIRData.decodedRawData == 0xE916FF00) {
      //Serial.println("Cleared EEPROM Stored Credentials");
      clearEEPROM();
      return 1;
    }
  }
  return 0;
}

/**
 * Interacts with the menu, catches keys pressed and performs corresponding
 * menu actions.
 */
void interactWithMenu() {
  if (irrecv.decode()) {
    // If in crypto display mode
    if (mode == 1) {
      // Down key pressed, select next element in menu
      if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
        coin_menu->moveDown();

      // Down key pressed, select previous element in menu
      if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
        coin_menu->moveUp();

      // OK pressed, see what button was pressed and take action
      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00) {
        char *button_action = coin_menu->press();

        if (button_action == "Edit Coin List") {
          in_coinlist = 1;
          coin_menu->getButtons()[1].drawSubMenu();
        }

        if (button_action == "Add New Coin") {
          coin_changer->setActive();
          coin_changer->display();
        }

        if (button_action == "Crypto Display Settings") {
          in_settings = 1;
          coin_menu->getButtons()[0].drawSubMenu();
        }

        if (button_action == "Exit Menu") {
          in_menu = 0;
          forceGetData(1); 
          updateAndDrawTime();

          displayNextCoin();
        }

        if (button_action == "Clear WiFi Credentials")
          clearEEPROM();
      }
      // If in Portfolio Mode
    } else if (mode == 2) {
      if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
        portfolio_menu->moveDown();

      if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
        portfolio_menu->moveUp();

      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00) {
        char *button_action = portfolio_menu->press();
        if (button_action == "Exit Menu") {
          in_menu = 0;

          portfolio->refreshSelectedCoins(); // Add any new coins to selected
          forceGetData(2);                   // Get data
          portfolio->addPriceToCandles();
          portfolio->display(); // Now display the portfolio
        }

        if (button_action == "Portfolio Settings") {
          in_settings = 1;
          portfolio_menu->getButtons()[0].drawSubMenu();
        }

        if (button_action == "Clear WiFi Credentials")
          clearEEPROM();

        if (button_action == "Edit Portfolio") {
          portfolio_editor->setActive();
          portfolio_editor->display();
        }
      }
    }
    delay(50);
    irrecv.resume();
  }
}

void drawMenu() {
  if (mode == 1) {
    coin_menu->display();
  } else if (mode == 2) {
    portfolio_menu->display();
  }
  drawTime();
}

/**
 * Interacts with the coin list submenu, catches keys pressed and performs
 * corresponding menu actions.
 */
void interactWithCoinList() {
  coin_menu->getButtons()[1].flashSelectedSelector();
  if (irrecv.decode()) {
    if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00) {
      char *action = coin_menu->getButtons()[1].pressSubMenu();
      if (action == "Return") {
        resetCoins();
        in_coinlist = 0;
        coin_menu->display();
      }
    }

    if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
      coin_menu->getButtons()[1].subMenuDown();

    if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
      coin_menu->getButtons()[1].subMenuUp();

    if (irrecv.decodedIRData.decodedRawData == 0xF708FF00)
      coin_menu->getButtons()[1].subMenuLeft();

    if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00)
      coin_menu->getButtons()[1].subMenuRight();

    delay(50);
    irrecv.resume();
  }
}

/**
 * Interacts with the settings sub menu, catches keys pressed and performs
 * corresponding menu actions.
 */
void interactWithSettings() {
  if (mode == 1) {
    coin_menu->getButtons()[0].flashSelectedSelector();
    if (irrecv.decode()) {
      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00) {
        char *action = coin_menu->getButtons()[0].pressSubMenu();
        if (action == "Return") {
          String str = coin_change_times
              [coin_menu->getButtons()[0].selectors[0].getSelected()[0]];
          coin_cycle_delay = str.toInt();

          str = coin_change_times
              [coin_menu->getButtons()[0].selectors[1].getSelected()[0]];
          coin_candle_update_delay = str.toInt();

          if (coin_candle_update_delay != last_coin_candle_update_delay)
            resetCoins();

          last_coin_candle_update_delay = coin_candle_update_delay;

          in_settings = 0;
          coin_menu->display();
        }
      }

      if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
        coin_menu->getButtons()[0].subMenuDown();

      if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
        coin_menu->getButtons()[0].subMenuUp();

      if (irrecv.decodedIRData.decodedRawData == 0xF708FF00)
        coin_menu->getButtons()[0].subMenuLeft();

      if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00)
        coin_menu->getButtons()[0].subMenuRight();

      delay(50);
      irrecv.resume();
    }
  } else if (mode == 2) {
    portfolio_menu->getButtons()[0].flashSelectedSelector();
    if (irrecv.decode()) {
      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00) {
        char *action = portfolio_menu->getButtons()[0].pressSubMenu();
        if (action == "Return") {
          String str = coin_change_times
              [portfolio_menu->getButtons()[0].selectors[0].getSelected()[0]];
          portfolio_candle_update_delay = str.toInt();

          if (portfolio_candle_update_delay !=
              last_portfolio_candle_update_delay)
            portfolio->clearCandles();

          last_portfolio_candle_update_delay = portfolio_candle_update_delay;

          in_settings = 0;
          portfolio_menu->display();
        }
      }

      if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
        portfolio_menu->getButtons()[0].subMenuDown();

      if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
        portfolio_menu->getButtons()[0].subMenuUp();

      if (irrecv.decodedIRData.decodedRawData == 0xF708FF00)
        portfolio_menu->getButtons()[0].subMenuLeft();

      if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00)
        portfolio_menu->getButtons()[0].subMenuRight();

      delay(50);
      irrecv.resume();
    }
  }
}
