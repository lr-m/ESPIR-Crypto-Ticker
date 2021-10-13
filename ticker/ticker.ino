#include <EEPROM.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <IRremote.h>
#include <ST7755_Keyboard.h>
#include <ST7755_Menu.h>
#include <ST7755_Crypto_Selector.h>
#include <Colours.h>

// Pull in bitmaps from bitmaps.c
extern unsigned char epd_bitmap_rocket_side [];
extern unsigned char epd_bitmap_fire [];
extern unsigned char epd_bitmap_small_fire [];

// Coin bitmaps
extern unsigned char epd_bitmap_bitcoin [];
extern unsigned char epd_bitmap_ethereum [];
extern unsigned char epd_bitmap_tether [];
extern unsigned char epd_bitmap_cardano [];
extern unsigned char epd_bitmap_binance [];
extern unsigned char epd_bitmap_xrp [];
extern unsigned char epd_bitmap_solana [];
extern unsigned char epd_bitmap_dot [];
extern unsigned char epd_bitmap_dogecoin [];
extern unsigned char epd_bitmap_terra [];
extern unsigned char epd_bitmap_litecoin [];
extern unsigned char epd_bitmap_avax [];
extern unsigned char epd_bitmap_algorand [];
extern unsigned char epd_bitmap_link [];
extern unsigned char epd_bitmap_matic [];
extern unsigned char epd_bitmap_tron [];
extern unsigned char epd_bitmap_cosmos [];
extern unsigned char epd_bitmap_tether [];
extern unsigned char epd_bitmap_bat [];
extern unsigned char epd_bitmap_nu [];
extern unsigned char epd_bitmap_uniswap [];

// Define PIN config
#define TFT_CS              D3
#define TFT_RST             D4                                            
#define TFT_DC              D2
#define TFT_SCLK            D5
#define TFT_MOSI            D7
#define RECV_PIN            D1

#define COIN_COUNT          20
#define MAX_SELECTED_COINS  8
#define CANDLE_COUNT        24
#define CANDLE_WIDTH        3

// Positional constants
#define NAME_START_2        98
#define NAME_START_3        92
#define NAME_START_4        86
#define NAME_START_5        80

#define PRICE_START_X       60
#define PRICE_START_Y       28

#define CHANGE_START_X      60
#define CHANGE_START_Y      50

#define COIN_MENU_BUTTON_COUNT   4
#define PORTFOLIO_MENU_BUTTON_COUNT 3

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, 
                        TFT_SCLK, TFT_RST);

// For interface
int current_candles = 1; // Current number of candles displayed
int selected_coins_count = 1; // Current number of coins selected
int current_coin; // Index of current coin being displayed
int current_second; // Current second from time NTP server
unsigned long last_minute = 0; // Last minute for time update check

// Flags
int ssid_entered; // Indicates if SSID of network entered
int password_entered; // Indicates if password of network entered
int network_initialised = 0; // Indicates if network has been initialised
int eeprom_address = 0; // Current address being written to on EEPROM
int in_menu = 0; // Indicates if currently in the menu
int in_settings = 0; // Indicates if currently in settings submenu
int in_coinlist = 0; // Indicates if currently in coin list submenu
int in_portfolio_editor = 0;
int time_slot_moved = 0; // Indicates if candles have been moved along

// For HTTP connection
WiFiClientSecure client;
HTTPClient http;
const char *fingerprint  = "33c57b69e63b765c393df1193b1768b81b0a1fd9";
char* url_start = "https://api.coingecko.com/api/v3/simple/price?ids=";
char* url_end = "&vs_currencies=gbp&include_24hr_change=true";

// For time retrieval
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
                              "Thursday", "Friday", "Saturday"};
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// IR remote
IRrecv irrecv(RECV_PIN);
decode_results results;

// Instantiate keyboard, menu, and portfolio editor
ST7755_Keyboard* keyboard;

ST7755_Menu* coin_menu;
ST7755_Menu* portfolio_menu;
char** coin_menu_button_functions;
char** portfolio_menu_button_functions;
char* coin_change_times[5] = {"5", "10", "15", "30", "60"};

Price_Selector* price_selector;

// For Portfolio
int* selected_portfolio_indexes;
int portfolio_values_selected = 0;
int selected_portfolio_index = 0;
int changing_amount = 0;

int curr_coin_list_index; // Index of the coin currently selected in coin list
char** coin_list; // List of all coin codes
int coin_cycle_delay; // Delay between coin changes
int candle_update_delay; // How often candles are pushed along (mins)
int last_candle_update_delay = 5; // To detect changes in candle update time

int mode = 1; // Coin or portfolio mode

// For candle graph element
typedef struct graph_candle {
  float opening;
  float closing;
  float high;
  float low;
} G_CANDLE;

// For storing coins
typedef struct coin {
  char* coin_code;
  char* coin_id;
  double current_price;
  double current_change;
  double amount;
  const unsigned char* bitmap;
  G_CANDLE* candles;
  uint16_t circle_colour;
  uint16_t bm_colour;
  uint16_t portfolio_colour;
} COIN;

COIN* coins; // List of coins that can be selected 
COIN** selected_coins; // List of pointers to coins currently selected

void setup(void) {
  Serial.begin(9600);
  irrecv.enableIRIn(); // Enable IR reciever

  // Initialise display
  tft.initR(INITR_GREENTAB); // Init ST7735S chip
  tft.setRotation(1);

  // Initialise keyboard
  keyboard = (ST7755_Keyboard*) malloc(sizeof(ST7755_Keyboard));
  *keyboard = ST7755_Keyboard(&tft);

  // Initialise portfolio amount selector
  price_selector = (Price_Selector*) malloc(sizeof(Price_Selector));
  *price_selector = Price_Selector(&tft);

  // Initialise coin menu
  coin_menu_button_functions = (char**) malloc(sizeof(char*) * 
                                                COIN_MENU_BUTTON_COUNT);
  coin_menu_button_functions[0] = "Settings";
  coin_menu_button_functions[1] = "Edit Coin List";
  coin_menu_button_functions[2] = "Clear WiFi Credentials";
  coin_menu_button_functions[3] = "Exit Menu";
  coin_menu = (ST7755_Menu*) malloc(sizeof(ST7755_Menu));
  *coin_menu = ST7755_Menu(&tft, COIN_MENU_BUTTON_COUNT, 
                            coin_menu_button_functions);

  // Initialise portfolio menu
  portfolio_menu_button_functions = (char**) malloc(sizeof(char*) * 
                                      PORTFOLIO_MENU_BUTTON_COUNT);
  portfolio_menu_button_functions[0] = "Edit Portfolio";
  portfolio_menu_button_functions[1] = "Clear WiFi Credentials";
  portfolio_menu_button_functions[2] = "Exit Menu";
  portfolio_menu = (ST7755_Menu*) malloc(sizeof(ST7755_Menu));
  *portfolio_menu = ST7755_Menu(&tft, PORTFOLIO_MENU_BUTTON_COUNT, 
                                portfolio_menu_button_functions);

  // Create coin array
  coins = (COIN*) malloc(sizeof(COIN) * COIN_COUNT);
  selected_coins = (COIN**) malloc(sizeof(COIN*) * MAX_SELECTED_COINS);
  
  coin_list = (char**) malloc(sizeof(char*) * COIN_COUNT);
  curr_coin_list_index = 0;

  coins[0] = *createCoin("BTC", "bitcoin", epd_bitmap_bitcoin, 
                          GOLD, WHITE, GOLD, 0);
  coins[1] = *createCoin("ETH", "ethereum", epd_bitmap_ethereum, 
                          WHITE, GRAY, GRAY, 0);
  coins[2] = *createCoin("LTC", "litecoin", epd_bitmap_litecoin, 
                          GRAY, WHITE, DARK_GREY, 0);
  coins[3] = *createCoin("ADA", "cardano", epd_bitmap_cardano, 
                          LIGHT_BLUE, WHITE, LIGHT_BLUE, 0);
  coins[4] = *createCoin("BNB", "binancecoin", epd_bitmap_binance, 
                          WHITE, GOLD, GOLD, 0);
  coins[5] = *createCoin("USDT", "tether", epd_bitmap_tether, 
                          GREEN, WHITE, GREEN, 0);
  coins[6] = *createCoin("XRP", "ripple", epd_bitmap_xrp, 
                          DARK_GREY, WHITE, DARK_GREY, 0);
  coins[7] = *createCoin("DOGE", "dogecoin", epd_bitmap_dogecoin, 
                          GOLD, WHITE, GOLD, 0);
  coins[8] = *createCoin("DOT", "polkadot", epd_bitmap_dot, 
                          WHITE, BLACK, WHITE, 0);
  coins[9] = *createCoin("SOL", "solana", epd_bitmap_solana, 
                          PINK, LIGHTNING_BLUE, PINK, 0);
  coins[10] = *createCoin("LUNA", "terra-luna", epd_bitmap_terra, 
                          NIGHT_BLUE, MOON_YELLOW, MOON_YELLOW, 0);
  coins[11] = *createCoin("ALGO", "algorand", epd_bitmap_algorand, 
                          WHITE, BLACK, WHITE, 0);
  coins[12] = *createCoin("LINK", "chainlink", epd_bitmap_link, 
                          DARK_BLUE, WHITE, DARK_BLUE, 0);
  coins[13] = *createCoin("MATIC", "matic-network", epd_bitmap_matic, 
                          PURPLE, WHITE, PURPLE, 0);
  coins[14] = *createCoin("TRX", "tron", epd_bitmap_tron, 
                          RED, WHITE, RED, 0);
  coins[15] = *createCoin("ATOM", "cosmos", epd_bitmap_cosmos, 
                          DARK_GREY, WHITE, DARK_GREY, 0);
  coins[16] = *createCoin("BAT", "basic-attention-token", epd_bitmap_bat, 
                          WHITE, SALMON, SALMON, 0);
  coins[17] = *createCoin("NU", "nucypher", epd_bitmap_nu, 
                          WHITE, LIGHT_BLUE, LIGHT_BLUE, 0);
  coins[18] = *createCoin("AVAX", "avalanche-2", epd_bitmap_avax, 
                          RED, WHITE, RED, 0);
  coins[19] = *createCoin("UNI", "uniswap", epd_bitmap_uniswap, 
                          WHITE, HOT_PINK, HOT_PINK, 0);

  // Add selectors to respective submenus
  coin_menu -> getButtons()[0].addSelector("Coin duration (secs):", 
                                          coin_change_times, 5, 1, 5);
  coin_menu -> getButtons()[0].addSelector("Candle duration (mins):", 
                                          coin_change_times, 5, 1, 5);
  coin_menu -> getButtons()[1].addSelector("Select up to 8 coins:", 
                                          coin_list, 4, MAX_SELECTED_COINS, 
                                          COIN_COUNT);

  // Get default coin delay time from selector
  String str = coin_change_times[coin_menu -> getButtons()[0].selectors[0].getSelected()[0]];
  coin_cycle_delay = str.toInt();

  // Get default update time from selector
  str = coin_change_times[coin_menu -> getButtons()[0].selectors[0].getSelected()[0]];
  candle_update_delay = str.toInt();

  // Try to read network credentials from EEPROM if they exist
  EEPROM.begin(512);
  byte value = EEPROM.read(0);
  
  if (value == 0){
    ssid_entered = 0;
    password_entered = 0;
    displayKeyboardInstructions();
    keyboard -> displayPrompt("Enter Network Name (SSID):");
    keyboard -> display();
  } else if (value == 1){
    ssid_entered = 1;
    password_entered = 1;
  }

  // Initialise portfolio arrays
  selected_portfolio_indexes = (int*) malloc(sizeof(int) * 9);

  for (int i = 0; i < 9; i++){
    selected_portfolio_indexes[i] = -1;
  }
}

void loop() {
  if (ssid_entered == 0){ // Get network name
      if (keyboard -> enterPressed() == 1){
        // Indicate SSID and pwd loaded into EEPROM
        EEPROM.write(eeprom_address, 1);
        eeprom_address++;
        writeToEEPROM(keyboard -> getCurrentInput());
        keyboard -> reset();
        
        ssid_entered = 1;
        
        keyboard -> displayPrompt("Enter Network Password:.");
      } else if (irrecv.decode()){
        interactWithKeyboard();
      }
    } else if (password_entered == 0){ // Get network password
      if (keyboard -> enterPressed() == 1){
        // Write password to EEPROM
        writeToEEPROM(keyboard -> getCurrentInput());
        
        password_entered = 1;
        keyboard -> reset();
      } else if (irrecv.decode()){
        interactWithKeyboard();
      }
    } else if (network_initialised == 0){ // Initialise network
      // If first value of eeprom is null terminator then take input from user
      char* loaded_ssid = (char*) malloc(sizeof(char) * 30);
      char* loaded_password = (char*) malloc(sizeof(char) * 30);

      // Give user a chance to wipe stored WiFi details (e.g. if network changes)
      tft.fillScreen(BLACK);
      tft.setCursor(5, 5);
      tft.write("Press * to clear EEPROM");

      for (int i = 0; i < 100; i++){
        delay(50);
        if (i < 33){
          tft.drawRect(map(i, 0, 99, 10, tft.width() - 10), 25, 2, 10, GREEN);
        } else if (i < 66){
          tft.drawRect(map(i, 0, 99, 10, tft.width() - 10), 25, 2, 10, GOLD);
        } else {
          tft.drawRect(map(i, 0, 99, 10, tft.width() - 10), 25, 2, 10, RED);
        }
        
        if (checkForCredsClear() == 1){
          password_entered = 0;
          ssid_entered = 0;
          return;
        }
      }

      loadCredsFromEEPROM(loaded_ssid, loaded_password);
      
      initialiseNetwork(loaded_ssid, loaded_password);

      // Only commit to EEPROM when correct credientials provided
      EEPROM.commit();

      // Initialise the candle for first coin (bitcoin)
      selected_coins[0] = coins;
      selected_coins[0] -> candles = (G_CANDLE*) malloc(sizeof(G_CANDLE) * 
                                                      CANDLE_COUNT);
      initialiseCandles(selected_coins[0] -> candles);

      forceGetData(1); // Get data for coin
    
      // Begin ndp time client and draw time on screen
      timeClient.begin();
      drawTime();

      //getBitmap("BTC");
    
      network_initialised = 1; // Indicate successful network init
    } else if (in_menu == 1) { // Do menu operations
      if (in_coinlist == 1){ // Interact with coin list submenu
        interactWithCoinList();
        return;
      }

      if (in_settings == 1){ // Interact with settings submenu
        interactWithSettings(); 
        return;
      }

      if (in_portfolio_editor == 1){ // Interact with portfolio editor
        interactWithPortfolioEditor();
        return;
      }
      
      interactWithMenu();
    } else { // Display crypto interface
      updateAndDrawTime();

      for (int i = 0; i < 50; i++){
        delay(10);
        if (irrecv.decode()){
          // Check if '#' pressed to open menu
          if (irrecv.decodedIRData.decodedRawData == 0xF20DFF00){
            Serial.println("Menu Opened");
            in_menu = 1;
            
            if (mode == 1){
              coin_menu -> display();
            } else if (mode == 2){
              portfolio_menu -> display();
            }
            
            delay(250);
            irrecv.resume();
            return;
          }

          // Change mode to opposite if up or down pressed
          if (irrecv.decodedIRData.decodedRawData == 0xE718FF00 || 
              irrecv.decodedIRData.decodedRawData == 0xAD52FF00){
            mode = mode == 1 ? 2 : 1;
            irrecv.resume();

            if (mode == 1){
              drawCoinObj(selected_coins[current_coin]);
            } else {
              drawPortfolio();
            }
            
            break;
          }

          // Check if user wants to move to next/previous coin
          if (mode == 1){
            // Previous coin
            if (irrecv.decodedIRData.decodedRawData == 0xF708FF00){
              if (current_coin == 0)
                current_coin = selected_coins_count;

              current_coin--;
    
              drawCoinObj(selected_coins[current_coin]);
            }
    
            // Next coin
            if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00){
              current_coin++;
              if (current_coin == selected_coins_count)
                current_coin = 0;
    
              drawCoinObj(selected_coins[current_coin]);
            }
          }
          
          irrecv.resume();
        }
      }

      // Move data along every x minutes
      if (timeClient.getMinutes() % candle_update_delay == 0 && 
          time_slot_moved == 0){
        Serial.println("Incremented Candles");
        for (int i = 0; i < selected_coins_count; i++){
          nextTimePeriod(selected_coins[i]);
        }
        current_candles = min(CANDLE_COUNT, current_candles+1);
        time_slot_moved = true;
      } else if (timeClient.getMinutes() % 5 != 0 && time_slot_moved == true){
        time_slot_moved = false;
      }
    
      // Update coin and portfolio data every 60 seconds
      if (current_second == 0){
        forceGetData(1);
        drawPortfolio();
        delay(1000);
      }

      // Move to next coin every *selected* seconds if in crypto mode
      if (mode == 1){
        if (current_second % coin_cycle_delay == 0){
          Serial.println("Moved to Next Coin");
          drawCoinObj(selected_coins[current_coin]);
      
          current_coin++;
          if (current_coin == selected_coins_count)
            current_coin = 0;
        }
      }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Portfolio /////// ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Draws the users portfolio on screen.
 */
void drawPortfolio(){
  tft.fillRect(0, 0, tft.width(), tft.height() - 15, BLACK);
  double* total_value = (double*) malloc(sizeof(double));
  *total_value = 0;
  updatePortfolio(total_value);

  // If in portfolio mode, draw the portfolio upon update
  if (mode == 2){
    tft.fillRect(0, 0, tft.width(), tft.height() - 12, BLACK);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.setCursor(5, 4);

    tft.print(char(156));
    tft.print(*total_value);

    if (selected_portfolio_indexes[0] != -1){
      double total = 0;
      int i = 0;
      double* coin_price_owned = (double*) malloc(sizeof(double) * 8);
      
      // Store cost of each owned coin
      while(selected_portfolio_indexes[i] != -1){
        coin_price_owned[i] = coins[selected_portfolio_indexes[i]].current_price * 
          coins[selected_portfolio_indexes[i]].amount;
        total += coins[selected_portfolio_indexes[i]].current_price * 
          coins[selected_portfolio_indexes[i]].amount;
        i++;
      }

      // Sort by price of owned coin (simple bubble sort)
      double temp = 0;
      for (int k = 0; k < i - 1; k++){
        for (int j = 0; j < i - k - 1; j++){
          if (coin_price_owned[j + 1] > coin_price_owned[j]){
            temp = selected_portfolio_indexes[j];
            selected_portfolio_indexes[j] = selected_portfolio_indexes[j + 1];
            selected_portfolio_indexes[j + 1] = temp;

            temp = coin_price_owned[j];
            coin_price_owned[j] = coin_price_owned[j+1];
            coin_price_owned[j + 1] = temp;
          }
        }
      }

      free(coin_price_owned);
    
      // Draw bar proportional to amount of coin owned
      i = 0;
      double coin_total = 0;
      double current_total = 0;
      while(selected_portfolio_indexes[i] != -1){
        coin_total = coins[selected_portfolio_indexes[i]].current_price * 
          coins[selected_portfolio_indexes[i]].amount;
        
        tft.fillRect(map(current_total, 0, total, 0, tft.width()), 24, 1 + 
          map(coin_total, 0, total, 0, tft.width()), 5, 
          coins[selected_portfolio_indexes[i]].portfolio_colour);
    
        current_total+=coin_total;
        i++;
      }
    
      // Draw coin code, cost of owned, and % of portfolio
      i = 0;
      while(selected_portfolio_indexes[i] != -1){
        coin_total = coins[selected_portfolio_indexes[i]].current_price * 
          coins[selected_portfolio_indexes[i]].amount;
        tft.setTextSize(1);
        tft.setCursor(7, 35 + i * 10);
        tft.fillRect(2, 34 + i * 10, 2, 10, 
          coins[selected_portfolio_indexes[i]].portfolio_colour);
        tft.print(coins[selected_portfolio_indexes[i]].coin_code);
        tft.setCursor(42, 35 + i * 10);
        tft.setTextColor(WHITE);
        tft.print(char(156));
        tft.print(coin_total);
        tft.setCursor(120, 35 + i * 10);
        tft.print(coin_total / total * 100, 1);
        tft.print("%");
        i++;
      }
    }
  }
  free(total_value);
}

/**
 * Updates the users portfolio and returns current total value.
 */
void updatePortfolio(double* total_value){
  Serial.println("Updating Portfolio");

  // Clear selected coins
  for (int i = 0; i < 8; i++){
    selected_portfolio_indexes[i] = -1;
  }

  // Add up to 8 non-zero coins to selected in portfolio
  int curr_selected_port_index = 0;
  for (int i = 0; i < COIN_COUNT; i++){
    if (coins[i].amount > 0){
      selected_portfolio_indexes[curr_selected_port_index] = i;
      curr_selected_port_index++;

      if (curr_selected_port_index == 8){
        break;
      }
    }
  }
  
  forceGetData(2);

  // Get the current total portfolio value
  int i = 0;
  while(selected_portfolio_indexes[i] != -1){
    *total_value += coins[selected_portfolio_indexes[i]].current_price * 
      coins[selected_portfolio_indexes[i]].amount;
    i++;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Data Retrieval Functions ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns the current time formatted as MM:SS
 */
String getFormattedTimeNoSeconds(NTPClient timeClient) {
  unsigned long rawTime = timeClient.getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  last_minute = minutes;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  return hoursStr + ":" + minuteStr;
}

/**
 * Attempts to get data from the coingecko api until it successfully gets it.
 */
void forceGetData(int app_mode){
  Serial.println("\nGetting Coin Data");
  while(getData(app_mode) == 0){
    Serial.println("Failed to get data, retrying...");
    delay(5000);
  }
  Serial.println("Successfully retrieved data");
}

// unsigned char* getBitmap(char* coin_id){
//   Serial.println("\nGetting Bitmap");
//   client.setFingerprint(fingerprint);
//   http.begin(client, "https://192.168.1.109:5000/api/v1/resources/coins?id=BTC");
  
//   int httpCode = http.GET();

//   if (httpCode > 0) {
//       StaticJsonDocument<800> doc;
//       // Get the request response payload (Price data)
//       DeserializationError error = deserializeJson(doc, http.getString());
//       Serial.println(http.getString());
//       if (error) {
//         Serial.print(F("deserializeJson() failed: "));
//         Serial.println(error.f_str());
//            //Close connection
//         http.end();   //Close connection
//         return NULL;
//       }

//       // Parameters
//       JsonObject current;
//       current = doc[coin_id];
//       String hello = current["bitmap"];
//       Serial.println(hello);
//   }
//   http.end();   //Close connection

//   return NULL;
// }

/**
 * Retrieves the current price, and 24hr change of the currently selected coins, 
 * uses CoinGecko API.
 */
int getData(int app_mode)
{
  // Instanciate Secure HTTP communication
  client.setFingerprint(fingerprint);

  char* url = constructURL(app_mode); // Get the URL
  
  http.begin(client, url);
  int httpCode = http.GET();

  free(url);

  if (httpCode > 0) {
      StaticJsonDocument<800> doc;
      // Get the request response payload (Price data)
      DeserializationError error = deserializeJson(doc, http.getString());
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        http.end();   //Close connection
        return 0;
      }
      
      JsonObject current;

      if (app_mode == 1){
        for (int i = 0; i < selected_coins_count; i++){
            current = doc[selected_coins[i] -> coin_id];
            selected_coins[i] -> current_price = current["gbp"];
            selected_coins[i] -> current_change = current["gbp_24h_change"];
            addPrice(selected_coins[i], incrementForGraph(current["gbp"])); 
        }
      } else if (app_mode == 2){
        int j = 0;
        while(selected_portfolio_indexes[j] != -1){
          current = doc[coins[selected_portfolio_indexes[j]].coin_id];
          coins[selected_portfolio_indexes[j]].current_price = current["gbp"];
          j++;
        }
      }
  }
  http.end();   //Close connection
  return 1;
}

/**
 * Constructs the URL for data retrieval.
 */
char* constructURL(int app_mode){
  char* url = (char*) malloc(sizeof(char) * 200);
  int curr_url_pos = 0;
  int i = 0;

  // Add start of url
  while(url_start[i] != 0){
    url[curr_url_pos] = url_start[i];
    curr_url_pos++;
    i++;
  }

  // Add coins selected in the specific coin part
  if (app_mode == 1){
    // Add all coins to url except last
    for (int j = 0; j < selected_coins_count - 1; j++){
      i = 0;
      while(selected_coins[j] -> coin_id[i] != 0){
        url[curr_url_pos] = selected_coins[j] -> coin_id[i];
        curr_url_pos++;
        i++;
      }
  
      url[curr_url_pos] = '%';
      url[curr_url_pos + 1] = '2';
      url[curr_url_pos + 2] = 'C';
      curr_url_pos += 3;
    }
  
    // Add final coin to url
    i = 0;
    while(selected_coins[selected_coins_count - 1] -> coin_id[i] != 0){
      url[curr_url_pos] = selected_coins[selected_coins_count-1] -> coin_id[i];
      curr_url_pos++;
      i++;
    }
  } else if (app_mode == 2){
    // Add all coins to url except last
    int j = 0;
    while (selected_portfolio_indexes[j] != -1){
      i = 0;
      while(coins[selected_portfolio_indexes[j]].coin_id[i] != 0){
        url[curr_url_pos] = coins[selected_portfolio_indexes[j]].coin_id[i];
        curr_url_pos++;
        i++;
      }
  
      url[curr_url_pos] = '%';
      url[curr_url_pos + 1] = '2';
      url[curr_url_pos + 2] = 'C';
      curr_url_pos += 3;

      j++;
    }
  }

  // Add ending to URL
  i = 0;
  while(url_end[i] != 0){
    url[curr_url_pos] = url_end[i];
    curr_url_pos++;
    i++;
  }
  url[curr_url_pos] = 0;
  return url;
}

/**
 * Initialises network and displays initialisation process + details.
 */ 
void initialiseNetwork(char* ssid, char* password){
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////// Coin Functions /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Creates a coin and returns pointer to it.
 */
COIN* createCoin(char* code, char* id, const unsigned char* bitmap, 
    uint16_t circle_col, uint16_t bm_col, uint16_t portfolio_col, double amount){
  COIN* new_coin = (COIN*) malloc(sizeof(COIN));
  new_coin -> coin_code = code;
  new_coin -> coin_id = id;
  new_coin -> bitmap = bitmap;
  new_coin -> circle_colour = circle_col;
  new_coin -> bm_colour = bm_col;
  new_coin -> amount = amount;
  new_coin -> portfolio_colour = portfolio_col;

  coin_list[curr_coin_list_index] = code;
  curr_coin_list_index++;
  
  return new_coin;
}

/**
 * Draws the passed coin object on the display
 */
void drawCoinObj(COIN* coin){
  tft.setTextColor(WHITE);
  tft.fillRect(0, 0, tft.width(), tft.height() - 12, BLACK);
  tft.fillCircle(12, 12, 44, coin -> circle_colour);
  drawBitmap(4, 4, coin -> bitmap, 40, 40, coin -> bm_colour);
  drawName(coin -> coin_code);
  drawPrice(coin -> current_price);
  drawPercentageChange(coin -> current_change);
  drawCandles(coin -> candles);
}

/**
 * Resets all of the coins (clears candles).
 */
void resetCoins(){
  // Free current candles
  for (int i = 0; i < selected_coins_count; i++){
    free(selected_coins[i] -> candles);
  }
  
  current_candles = 1;
  selected_coins_count = 0;
  current_coin = 0;

  // Get all selected coins
  Serial.print("\nCoins Selected: ");
  for (int i = 0; i < MAX_SELECTED_COINS; i++){
    if (coin_menu -> getButtons()[1].selectors[0].getSelected()[i] == -1)
      break;
    
    Serial.print((coins + coin_menu -> getButtons()[1].selectors[0].getSelected()[i]) -> coin_id);
    Serial.print(", ");
    selected_coins_count++;
    selected_coins[i] = coins + coin_menu -> getButtons()[1].selectors[0].getSelected()[i];
    selected_coins[i] -> candles = (G_CANDLE*) malloc(sizeof(G_CANDLE) * CANDLE_COUNT);
    initialiseCandles(selected_coins[i] -> candles);
  }
  
  forceGetData(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Candle Graph Functions /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialises candle array.
 */
G_CANDLE initialiseCandles(G_CANDLE* candle_arr){
  for (int i = 0; i < CANDLE_COUNT; i++){
    candle_arr[i].opening = -1;
    candle_arr[i].closing = -1;
    candle_arr[i].low = -1;
    candle_arr[i].high = -1;
  }
}

/**
 * Draws the candle chart onto the display.
 */
void drawCandles(G_CANDLE* candles){
  double max_val = -1;
  double min_val = 99999999;
  
  for (int i = CANDLE_COUNT - current_candles; i < CANDLE_COUNT; i++){
    if (candles[i].high > max_val){
      max_val = candles[i].high;
    }

    if (candles[i].low < min_val){
      min_val = candles[i].low;
    }
  }

  int graph_top = tft.height()/2;
  int graph_bottom = tft.height()-22;
  
  for (int i = CANDLE_COUNT - current_candles; i < CANDLE_COUNT; i++){
    int opening_y;
    int closing_y;
    int high_y;
    int low_y;

    if (min_val != max_val){
      opening_y = (candles[i].opening - min_val) * ((double) graph_top - 
        (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
      closing_y = (candles[i].closing - min_val) * ((double) graph_top - 
        (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
      high_y = (candles[i].high - min_val) * ((double) graph_top - 
        (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
      low_y = (candles[i].low - min_val) * ((double) graph_top - 
        (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
    } else {
      opening_y = ((tft.height() / 2) + (tft.height() - 5)) / 2;
      closing_y = ((tft.height() / 2) + (tft.height() - 5)) / 2;
      high_y = ((tft.height() / 2) + (tft.height() - 5)) / 2;
      low_y = ((tft.height() / 2) + (tft.height() - 5)) / 2;
    }

    int bar_x = map(i, 0, CANDLE_COUNT, 2, tft.width()-2);
    uint16_t color; 

    if (candles[i].opening > candles[i].closing){
      color = RED;
    } else {
      color = ST77XX_GREEN;
    }

    tft.drawLine(bar_x, high_y, bar_x + 2, high_y, color);
    tft.drawLine(bar_x + 1, high_y, bar_x + 1, min(opening_y, closing_y), color);
    tft.fillRect(bar_x, min(opening_y, closing_y), CANDLE_WIDTH, 
      max(opening_y, closing_y) - min(opening_y, closing_y), color);
    tft.drawLine(bar_x + 1, low_y, bar_x + 1, max(opening_y, closing_y), color);
    tft.drawLine(bar_x, low_y, bar_x + 2, low_y, color);
  }
}

/**
 * Increments small values to larger values for the candle graph (to get by 
 * 2dp default precision).
 */
float incrementForGraph(double val){
  float to_incr = (float) val;
  while(to_incr < 10000){
    to_incr *= 10;
  }
  return to_incr;
}

/**
 * Adds the given price to the candle chart of the given coin.
 */
void addPrice(COIN* coin, float val){
  if (coin -> candles[CANDLE_COUNT - 1].opening == -1){
    coin -> candles[CANDLE_COUNT - 1].opening = val;
    coin -> candles[CANDLE_COUNT - 1].closing = val;
    coin -> candles[CANDLE_COUNT - 1].high = val;
    coin -> candles[CANDLE_COUNT - 1].low = val;
  }

  coin -> candles[CANDLE_COUNT-1].closing = val;

  if (val > coin -> candles[CANDLE_COUNT-1].high)
    coin -> candles[CANDLE_COUNT-1].high = val;

  if (val < coin -> candles[CANDLE_COUNT - 1].low)
    coin -> candles[CANDLE_COUNT - 1].low = val;
}

/**
 * Moves all of the candles down to the next time period index and initialies 
 * new time period.
 */
void nextTimePeriod(COIN* coin){
  coin -> candles[CANDLE_COUNT - 1].closing = incrementForGraph(coin -> current_price);

  for (int i = 1; i < CANDLE_COUNT; i++){
    coin -> candles[i - 1].opening = coin -> candles[i].opening;
    coin -> candles[i - 1].closing = coin -> candles[i].closing;
    coin -> candles[i - 1].high = coin -> candles[i].high;
    coin -> candles[i - 1].low = coin -> candles[i].low;
  }

  coin -> candles[CANDLE_COUNT - 1].opening = incrementForGraph(coin -> current_price);
  coin -> candles[CANDLE_COUNT - 1].high = incrementForGraph(coin -> current_price);
  coin -> candles[CANDLE_COUNT - 1].low = incrementForGraph(coin -> current_price);
  coin -> candles[CANDLE_COUNT - 1].closing = incrementForGraph(coin -> current_price);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// Keyboard  Functions ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Displays the instructions of how to use the keyboard to enter details.
 */
void displayKeyboardInstructions(){
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.println("Keyboard Instructions");
  tft.setTextColor(RED);
  tft.println("\n*To see again, unplug device and plug it back in*\n");
  tft.setTextColor(WHITE);
  tft.println("<-- : Select Key on Left");
  tft.println("--> : Select Key on Right");
  tft.println("DOWN: Enter Bottom Row");
  tft.println("UP  : Exit Bottom Row\n");
  tft.println("1   : Lower Case");
  tft.println("2   : Upper Case");
  tft.println("3   : Numbers");
  tft.println("4   : Special Characters");
  delay(5000);
}

/**
 * Decodes the pressed key and performs the corresponding action on the 
 * keyboard.
 */
void interactWithKeyboard(){
  // OK
  if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00)
    keyboard -> press();

  // 1
  if (irrecv.decodedIRData.decodedRawData == 0xBA45FF00)
    keyboard -> setMode(0);

  // 2
  if (irrecv.decodedIRData.decodedRawData == 0xB946FF00)
    keyboard -> setMode(1);

  // 3
  if (irrecv.decodedIRData.decodedRawData == 0xB847FF00)
    keyboard -> setMode(2);

  // 4
  if (irrecv.decodedIRData.decodedRawData == 0xBB44FF00)
    keyboard -> setMode(3);

  // DOWN
  if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
    keyboard -> goToTabs();

  // UP
  if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
    keyboard -> exitTabs();

  // RIGHT
  if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00)
    keyboard -> moveRight();

  // LEFT
  if (irrecv.decodedIRData.decodedRawData == 0xF708FF00)
    keyboard -> moveLeft();
  
  irrecv.resume();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// Interface Functions ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Draws the current time and day on the display.
 */
void drawTime(){
  tft.fillRect(0, tft.height()-10, (tft.width()/2)-20, 10, BLACK);
  tft.fillRect((tft.width()/2)+20, tft.height()-10, (tft.width()/2)-20, 10, BLACK);

  tft.setCursor(3, tft.height()-10);
  tft.setTextSize(1);

  tft.print(daysOfTheWeek[timeClient.getDay()]);
  tft.setCursor(tft.width()-32, tft.height()-10);
  tft.print(getFormattedTimeNoSeconds(timeClient));

//  drawBitmap((tft.width()/2)+4, tft.height()-12, epd_bitmap_rocket_side, 16, 12, WHITE);
//  drawBitmap((tft.width()/2)-10, tft.height()-12, epd_bitmap_fire, 14, 12, RED);
//  drawBitmap((tft.width()/2)-5, tft.height()-10, epd_bitmap_small_fire, 9, 8, GOLD);
}


/**
 * Updates and draws the current time.
 */
void updateAndDrawTime(){
  // Check minutes and update time
  while(!timeClient.update()){delay(500);}
  
  current_second = timeClient.getSeconds();

  // Draw time if a minute has passed since last drawn
  if (last_minute != timeClient.getMinutes()){
    Serial.println("Time Updated on Screen");
    drawTime();
  }
}

/**
 * Draws the name of the coin on the screen.
 */
void drawName(String coin_name){
  tft.setTextSize(2);
  if (coin_name.length() == 3){
    tft.setCursor(NAME_START_3, 6);
  } else if (coin_name.length() == 4) {
    tft.setCursor(NAME_START_4, 6);
  } else if (coin_name.length() == 5){
    tft.setCursor(NAME_START_5, 6);
  } else {
    tft.setCursor(NAME_START_2, 6);
  }
  tft.print(coin_name);
}

/**
 * Draws the current price of coin on the screen.
 */
void drawPrice(float price){
  tft.setCursor(PRICE_START_X, PRICE_START_Y);

  String print_price;

  if (price/10000 > 1){
    print_price = String(price, 1);
  } else if (price/1000 > 1){
    print_price = String(price, 2);
  } else if (price/100 > 1){
    print_price = String(price, 3);
  } else if (price/10 > 1){
    print_price = String(price, 4);
  } else {
    print_price = String(price, 5);
  }

  tft.print(char(156));
  tft.print(print_price);
}

/**
 * Draws the percentage change on the screen.
 */
void drawPercentageChange(float change){
  tft.setCursor(CHANGE_START_X, CHANGE_START_Y);
  tft.setTextSize(1);
  tft.print("24 Hour: ");
  if (change < 0){
    tft.setTextColor(RED);
  } else {
    tft.setTextColor(ST77XX_GREEN);
    tft.print('+');
  }
  tft.print(change);
  tft.print('%');
  tft.setTextColor(WHITE);
}

/**
 * Draws a passed bitmap on the screen at the given position with the given 
 * colour.
 */
void drawBitmap(int16_t x, int16_t y,const uint8_t *bitmap, int16_t w, 
    int16_t h, uint16_t color) {
  int16_t i, j, byteWidth = (w + 7) / 8;
  uint8_t byte;
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++) {
      if(i & 7) byte <<= 1;
      else      byte   = pgm_read_byte(bitmap + j * byteWidth + i / 8);
      if(byte & 0x80) tft.drawPixel(x+i, y+j, color);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// EEPROM Functions ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Clears the EEPROM memory.
 */
void clearEEPROM(){
  for (int i = 0; i < 512; i++){
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

/**
 * Prints the contents of the EEPROM to serial.
 */
void printEEPROM(){
  // Clear EEPROM
  for (int i = 0; i < 512; i++){
    Serial.print(EEPROM.read(i));
    Serial.print(" ");
  }
}

/**
 * Writes a string to the EEPROM.
 */
void writeToEEPROM(char* input){
  int i = 0;
  do{
    EEPROM.write(eeprom_address, input[i]);
    eeprom_address++;
    i++;
  } while(input[i] != 0);

  EEPROM.write(eeprom_address, '\0');
  eeprom_address++;
}

/**
 * Loads the credits from the EEPROM, the format is {0/1}*NAME*\0*PASSWORD*.
 */
void loadCredsFromEEPROM(char* ssid, char* pass){
  int addr = 1;

  // Read SSID
  byte read_from_eeprom = EEPROM.read(addr);
  int i = 0;
  while(read_from_eeprom != 0){
    ssid[i] = (char) read_from_eeprom;
    addr++;
    i++;
    read_from_eeprom = EEPROM.read(addr);
  }
  ssid[i] = 0;
  addr++;

  // Read Password
  read_from_eeprom = EEPROM.read(addr);
  i = 0;
  while(read_from_eeprom != 0){
    pass[i] = (char) read_from_eeprom;
    addr++;
    i++;
    read_from_eeprom = EEPROM.read(addr);
  }
  pass[i] = 0;
}

/**
 * Check if '*' pressed, if so, clears existing WiFi credentials.
 */
int checkForCredsClear(){
  if (irrecv.decode()){
    if (irrecv.decodedIRData.decodedRawData == 0xE916FF00){
      Serial.println("Cleared EEPROM Stored Credentials");
      clearEEPROM();
      return 1;
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Menu Interaction ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Interacts with the menu, catches keys pressed and performs corresponding 
 * menu actions.
 */
void interactWithMenu(){
  if (irrecv.decode()){
    if (mode == 1){
      if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
        coin_menu -> moveDown();

      if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
        coin_menu -> moveUp();

      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00){
        char* button_action = coin_menu -> press();
        if (button_action == "Exit Menu"){
          in_menu = 0;
          drawCoinObj(selected_coins[current_coin]);
          
          current_coin++;
          if (current_coin == selected_coins_count){
            current_coin = 0;
          }
          drawTime();
        }

        if (button_action == "Clear WiFi Credentials"){
          clearEEPROM();
        }

        if (button_action == "Settings"){
          in_settings = 1;
          coin_menu -> getButtons()[0].drawSubMenu();
        }

        if (button_action == "Edit Coin List"){
          in_coinlist = 1;
          coin_menu -> getButtons()[1].drawSubMenu();
        }
      }
    } else if (mode == 2){
      if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
        portfolio_menu -> moveDown();

      if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
        portfolio_menu -> moveUp();

      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00){
        char* button_action = portfolio_menu -> press();
        if (button_action == "Exit Menu"){
          in_menu = 0;
          drawPortfolio();
        }

        if (button_action == "Clear WiFi Credentials"){
          clearEEPROM();
        }

        if (button_action == "Edit Portfolio"){
          in_portfolio_editor = 1;
          drawPortfolioEditor();
        }
      }
    }
    delay(50);
    irrecv.resume();
  }
}

/**
 * Draws the Portfolio Editor.
 */
void drawPortfolioEditor(){
  tft.fillRect(0, 0, tft.width(), tft.height() - 12, BLACK);
  tft.setTextColor(WHITE);
  int i = 0;
  while(i < COIN_COUNT){
    tft.fillRect(map(i % 4, 0, 3, 2, tft.width() - 43), map(floor(i / 4), 0, 5, 10, 
      tft.height() / 2), 4, 7, coins[i].portfolio_colour);
    tft.setCursor(map(i % 4, 0, 3, 9, tft.width() - 36), map(floor(i / 4), 0, 5, 10, 
      tft.height() / 2));
    tft.print(coins[i].coin_code);
    i++;
  }

  tft.fillRect(map(selected_portfolio_index % 4, 0, 3, 9, tft.width() - 36) - 1, 
    map(floor(selected_portfolio_index / 4), 0, 5, 10, tft.height() / 2) - 1, 31, 9, GRAY);
  tft.setCursor(map(selected_portfolio_index % 4, 0, 3, 9, tft.width() - 36), 
    map(floor(selected_portfolio_index / 4), 0, 5, 10, tft.height() / 2));
  tft.setTextColor(BLACK);
  tft.print(coins[selected_portfolio_index].coin_code);
}

void drawMenu(){
  if (mode == 1){
    coin_menu -> display();
  } else if (mode == 2){
    portfolio_menu -> display();
  }
}

/**
 * Interacts with the portfolio editor.
 */
void interactWithPortfolioEditor(){
  if (irrecv.decode()){

    if (irrecv.decodedIRData.decodedRawData == 0xF20DFF00){
      in_portfolio_editor = 0;
      irrecv.resume();
      drawMenu();
      return;
    }
    
    if (changing_amount == 0){
      tft.setTextSize(1);
      tft.fillRect(map(selected_portfolio_index % 4, 0, 3, 9, tft.width() - 36) - 1, 
        map(floor(selected_portfolio_index / 4), 0, 5, 10, tft.height() / 2) - 1, 31, 9, BLACK);
      tft.setTextColor(WHITE);
      tft.setCursor(map(selected_portfolio_index % 4, 0, 3, 9, tft.width() - 36), 
        map(floor(selected_portfolio_index / 4), 0, 5, 10, tft.height() / 2));
      tft.print(coins[selected_portfolio_index].coin_code);
      
      // left
      if (irrecv.decodedIRData.decodedRawData == 0xF708FF00){
        selected_portfolio_index = selected_portfolio_index == 0 ? COIN_COUNT-1 
          : selected_portfolio_index - 1;
      }
  
      //right
      if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00){
        selected_portfolio_index = selected_portfolio_index == COIN_COUNT - 1 ? 0 
          : selected_portfolio_index + 1;
      }
  
      //ok
      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00){
        price_selector -> setValue(coins[selected_portfolio_index].amount);
        price_selector -> display();
        changing_amount = 1;
      }

      tft.setTextSize(1);
      tft.fillRect(map(selected_portfolio_index % 4, 0, 3, 9, tft.width() - 36) - 1, 
        map(floor(selected_portfolio_index / 4), 0, 5, 10, tft.height() / 2) - 1, 31, 9, GRAY);
      tft.setCursor(map(selected_portfolio_index % 4, 0, 3, 9, tft.width() - 36), 
        map(floor(selected_portfolio_index / 4), 0, 5, 10, tft.height() / 2));
      tft.setTextColor(BLACK);
      tft.print(coins[selected_portfolio_index].coin_code);
    } else {
      // left
      if (irrecv.decodedIRData.decodedRawData == 0xF708FF00){
        price_selector ->  increaseValueToAdd();
        price_selector -> redrawValueChange();
      }
  
      //right
      if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00){
        price_selector -> decreaseValueToAdd();
        price_selector -> redrawValueChange();
      }

      //up
      if (irrecv.decodedIRData.decodedRawData == 0xE718FF00){
        price_selector -> addValue();
        price_selector -> redrawValue();
      }

      //down
      if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00){
        price_selector -> subValue();
        price_selector -> redrawValue();
      }

      //ok
      if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00){
        changing_amount = 0;
        coins[selected_portfolio_index].amount = price_selector -> getValue();
        price_selector -> clear();
      }
    }
    delay(100);
    irrecv.resume();
  }
}

/**
 * Interacts with the coin list submenu, catches keys pressed and performs 
 * corresponding menu actions.
 */
void interactWithCoinList(){
  coin_menu -> getButtons()[1].flashSelectedSelector();
  if (irrecv.decode()){
    if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00){
      char* action = coin_menu -> getButtons()[1].pressSubMenu();
      if (action == "Return"){
        resetCoins();
        in_coinlist = 0;
        coin_menu -> display();
      }
    }

    if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
      coin_menu -> getButtons()[1].subMenuDown();

    if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
      coin_menu -> getButtons()[1].subMenuUp();

    if (irrecv.decodedIRData.decodedRawData == 0xF708FF00)
      coin_menu -> getButtons()[1].subMenuLeft();

    if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00)
      coin_menu -> getButtons()[1].subMenuRight();
    
    delay(50);
    irrecv.resume();
  }
}

/**
 * Interacts with the settings sub menu, catches keys pressed and performs corresponding menu actions.
 */
void interactWithSettings(){
  coin_menu -> getButtons()[0].flashSelectedSelector();
  if (irrecv.decode()){
    if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00){
      char* action = coin_menu -> getButtons()[0].pressSubMenu();
      if (action == "Return"){
        String str = coin_change_times[coin_menu -> getButtons()[0].selectors[0].getSelected()[0]];
        coin_cycle_delay = str.toInt();

        str = coin_change_times[coin_menu -> getButtons()[0].selectors[1].getSelected()[0]];
        candle_update_delay = str.toInt();

        Serial.print(candle_update_delay);
        Serial.print(" ");
        Serial.println(last_candle_update_delay);

        if (candle_update_delay != last_candle_update_delay)
          resetCoins();

        last_candle_update_delay = candle_update_delay;
        
        in_settings = 0;
        coin_menu -> display();
      }
    }

    if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00)
      coin_menu -> getButtons()[0].subMenuDown();

    if (irrecv.decodedIRData.decodedRawData == 0xE718FF00)
      coin_menu -> getButtons()[0].subMenuUp();

    if (irrecv.decodedIRData.decodedRawData == 0xF708FF00)
      coin_menu -> getButtons()[0].subMenuLeft();

    if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00)
      coin_menu -> getButtons()[0].subMenuRight();

    delay(50);
    irrecv.resume();
  }
}
