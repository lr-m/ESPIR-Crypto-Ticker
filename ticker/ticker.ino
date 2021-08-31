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
#include <Colours.h>

// Pull in bitmaps from bitmaps.c
extern unsigned char epd_bitmap_bitcoin [];
extern unsigned char epd_bitmap_Ethereum [];
extern unsigned char epd_bitmap_cardano [];
extern unsigned char epd_bitmap_algorand [];
extern unsigned char epd_bitmap_link [];
extern unsigned char epd_bitmap_xrp [];
extern unsigned char epd_bitmap_matic [];
extern unsigned char epd_bitmap_tron [];
extern unsigned char epd_bitmap_cosmos [];
extern unsigned char epd_bitmap_binance [];
extern unsigned char epd_bitmap_tether [];
extern unsigned char epd_bitmap_bat [];

#define CG_URL "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin%2Cethereum%2Ccardano%2Calgorand%2Cchainlink%2Cripple%2Cmatic-network%2Ctron%2Ccosmos%2Cbinancecoin%2Ctether%2Cbasic-attention-token&vs_currencies=gbp&include_24hr_change=true"

// Define PIN config
#define TFT_CS         D3
#define TFT_RST        D4                                            
#define TFT_DC         D2
#define TFT_SCLK       D5
#define TFT_MOSI       D7
#define RECV_PIN       D1

#define COIN_COUNT 12
#define CANDLE_COUNT 24
#define CANDLE_WIDTH 3

// Positional constants
#define NAME_START_3 94
#define NAME_START_4 86
#define NAME_START_5 82

#define PRICE_START_X 68
#define PRICE_START_Y 28

#define CHANGE_START_X 56
#define CHANGE_START_Y 50

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// For interface
int current_candles = 1;
int current_coin;
int current_second;
int last_minute = 0;
int time_slot_moved = 0;

// Flags
int ssid_entered;
int password_entered;
int network_initialised = 0;
int eeprom_address = 0;

// For HTTP connection
WiFiClientSecure client;
HTTPClient http;  //Object of class HTTPClient
const char *fingerprint  = "33c57b69e63b765c393df1193b1768b81b0a1fd9";
String payload = "{}";

// For time retrieval
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// IR remote
IRrecv irrecv(RECV_PIN);
decode_results results;

// Instantiate keyboard class
ST7755_Keyboard* keyboard;

// For candle graph
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
  const unsigned char* bitmap;
  G_CANDLE* candles;
  uint16_t circle_colour;
  uint16_t bm_colour;
} COIN;

COIN* coins;

void setup(void) {
  Serial.begin(9600);
  irrecv.enableIRIn();

  tft.initR(INITR_GREENTAB);      // Init ST7735S chip
  tft.setRotation(1);

  keyboard = (ST7755_Keyboard*) malloc(sizeof(ST7755_Keyboard));
  *keyboard = ST7755_Keyboard(&tft);

  EEPROM.begin(512);

  byte value = EEPROM.read(0);
  if (value == 0){
    ssid_entered = 0;
    password_entered = 0;
    displayKeyboardInstructions();
    keyboard->displayPrompt("Enter Network Name (SSID):");
    keyboard->display();
  } else if (value == 1){
    ssid_entered = 1;
    password_entered = 1;
  }
}

// Update time every minute
// Update data every 30 seconds
// Update crypto displayed every 5 seconds

void loop() {
  if (ssid_entered == 0){
      if (keyboard->enterPressed() == 1){
        // Indicate SSID and pwd loaded into EEPROM
        EEPROM.write(eeprom_address, 1);
        eeprom_address++;
        
        writeToEEPROM(keyboard->getCurrentInput());
        
        ssid_entered = 1;
        keyboard->reset();
        keyboard->displayPrompt("Enter Network Password:");
      } else if (irrecv.decode()){
        interactWithKeyboard();
      }
    } else if (password_entered == 0){
      if (keyboard->enterPressed() == 1){
        // Write password to EEPROM
        writeToEEPROM(keyboard->getCurrentInput());
        
        password_entered = 1;
        keyboard->reset();
      } else if (irrecv.decode()){
        interactWithKeyboard();
      }
    } else if (network_initialised == 0){
      // If first value of eeprom is null terminator then take input from user
      char* loaded_ssid = (char*) malloc(sizeof(char)*30);
      char* loaded_password = (char*) malloc(sizeof(char)*30);

      loadCredsFromEEPROM(loaded_ssid, loaded_password);
    
      tft.fillScreen(BLACK);
      tft.setTextSize(1);
      tft.setTextColor(WHITE);
      tft.setCursor(0, 10);
      tft.println(" SSID: ");
      tft.setTextColor(RED);
      tft.println(" " + String(loaded_ssid));
      tft.setTextColor(WHITE);
      tft.println("\n Password: ");
      tft.setTextColor(RED);
      tft.println(" " + String(loaded_password));
      tft.setTextColor(WHITE);
      WiFi.begin(loaded_ssid, loaded_password);
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

      // Only commit to EEPROM when correct credientials provided
      EEPROM.commit();
    
      // Create coin array
      coins = (COIN*) malloc(sizeof(COIN)*COIN_COUNT);
    
      coins[0] = *createCoin("BTC", "bitcoin", epd_bitmap_bitcoin, GOLD, WHITE);
      coins[1] = *createCoin("ETH", "ethereum", epd_bitmap_Ethereum, WHITE, GRAY);
      coins[2] = *createCoin("ADA", "cardano", epd_bitmap_cardano, LIGHT_BLUE, WHITE);
      coins[3] = *createCoin("ALGO", "algorand", epd_bitmap_algorand, WHITE, BLACK);
      coins[4] = *createCoin("LINK", "chainlink", epd_bitmap_link, DARK_BLUE, WHITE);
      coins[5] = *createCoin("XRP", "ripple", epd_bitmap_xrp, DARK_GREY, WHITE);
      coins[6] = *createCoin("MATIC", "polygon", epd_bitmap_matic, PURPLE, WHITE);
      coins[7] = *createCoin("TRX", "tron", epd_bitmap_tron, RED, WHITE);
      coins[8] = *createCoin("ATOM", "cosmos", epd_bitmap_cosmos, DARK_GREY, WHITE);
      coins[9] = *createCoin("BNB", "binance", epd_bitmap_binance, WHITE, GOLD);
      coins[10] = *createCoin("USDT", "tether", epd_bitmap_tether, GREEN, WHITE);
      coins[11] = *createCoin("BAT", "basic-attention-token", epd_bitmap_bat, WHITE, SALMON);

      getData();
      delay(1000);
    
      // Begin ndp time client
      timeClient.begin();
      timeClient.update();
    
      // Retrieve current market data
      network_initialised = 1;
      drawTime();
    } else {
        if (irrecv.decode()){
          Serial.println(irrecv.decodedIRData.decodedRawData, HEX);

          if (irrecv.decodedIRData.decodedRawData == 0xE916FF00){
            clearEEPROM();
            Serial.println("EEPROM cleared");
          }
          
          irrecv.resume();
        }
        
        // Check minutes and update time
        last_minute = timeClient.getMinutes();
        timeClient.update();
        Serial.print(timeClient.getMinutes());
        Serial.print(":");
        Serial.println(timeClient.getSeconds());
        delay(500);
        current_second = timeClient.getSeconds();
      
        // Draw time if a minute has passed since last drawn
        if (last_minute != timeClient.getMinutes()){
          Serial.println("Time Drawn");
          drawTime();
          last_minute = timeClient.getMinutes();
        }
      
        // Move data along every 5 minutes
        if (timeClient.getMinutes()%5 == 0 && time_slot_moved == 0){
          Serial.println("Candles increased");
          for (int i = 0; i < COIN_COUNT; i++){
            nextTimePeriod(coins+i);
          }
          current_candles = min(CANDLE_COUNT, current_candles+1);
          time_slot_moved = true;
        } else if (timeClient.getMinutes()%5 != 0 && time_slot_moved == 1){
          time_slot_moved = false;
        }
      
        // Update data every 30 seconds
        if (current_second%30 == 0){
          Serial.println("Updating data");
          getData();
        }
      
        // Move to next coin every 6 seconds
        if (current_second%6 == 0){
          Serial.println("Next coin");
          drawCoinObj(coins+current_coin);
      
          current_coin++;
          if (current_coin == COIN_COUNT){
            current_coin = 0;
          }
        }
    }
}

void interactWithKeyboard(){
  // OK
  if (irrecv.decodedIRData.decodedRawData == 0xE31CFF00){
    keyboard->press();
  }

  // 1
  if (irrecv.decodedIRData.decodedRawData == 0xBA45FF00){
    keyboard->setMode(0);
  }

  // 2
  if (irrecv.decodedIRData.decodedRawData == 0xB946FF00){
    keyboard->setMode(1);
  }

  // 3
  if (irrecv.decodedIRData.decodedRawData == 0xB847FF00){
    keyboard->setMode(2);
  }

  // 4
  if (irrecv.decodedIRData.decodedRawData == 0xBB44FF00){
    keyboard->setMode(3);
  }

  // DOWN
  if (irrecv.decodedIRData.decodedRawData == 0xAD52FF00){
    keyboard->goToTabs();
  }

  // UP
  if (irrecv.decodedIRData.decodedRawData == 0xE718FF00){
    keyboard->exitTabs();
  }

  // RIGHT
  if (irrecv.decodedIRData.decodedRawData == 0xA55AFF00){
    keyboard->moveRight();
  }

  // LEFT
  if (irrecv.decodedIRData.decodedRawData == 0xF708FF00){
    keyboard->moveLeft();
  }
  irrecv.resume();
}

void drawCoinObj(COIN* coin){
  tft.setTextColor(WHITE);
  tft.fillRect(0, 0, tft.width(), tft.height()-20, BLACK);
  tft.fillCircle(12, 12, 44, coin->circle_colour);
  drawBitmap(4, 4, coin->bitmap, 40, 40, coin->bm_colour);
  drawName(coin->coin_code);
  drawPrice(coin->current_price);
  drawPercentageChange(coin->current_change);
  drawCandles(coin->candles);
}

void addPrice(COIN* coin, float val){
  if (coin->candles[CANDLE_COUNT-1].opening == -1){
    coin->candles[CANDLE_COUNT-1].opening = val;
    coin->candles[CANDLE_COUNT-1].closing = val;
    coin->candles[CANDLE_COUNT-1].high = val;
    coin->candles[CANDLE_COUNT-1].low = val;
  }

  coin->candles[CANDLE_COUNT-1].closing = val;

  if (val > coin->candles[CANDLE_COUNT-1].high){
    coin->candles[CANDLE_COUNT-1].high = val;
  }

  if (val < coin->candles[CANDLE_COUNT-1].low){
    coin->candles[CANDLE_COUNT-1].low = val;
  }
}

void nextTimePeriod(COIN* coin){
  coin->candles[CANDLE_COUNT-1].closing = incrementForGraph(coin->current_price);

  for (int i = 1; i < CANDLE_COUNT; i++){
    coin->candles[i-1].opening = coin->candles[i].opening;
    coin->candles[i-1].closing = coin->candles[i].closing;
    coin->candles[i-1].high = coin->candles[i].high;
    coin->candles[i-1].low = coin->candles[i].low;
  }

  coin->candles[CANDLE_COUNT-1].opening = incrementForGraph(coin->current_price);
  coin->candles[CANDLE_COUNT-1].high = incrementForGraph(coin->current_price);
  coin->candles[CANDLE_COUNT-1].low = incrementForGraph(coin->current_price);
  coin->candles[CANDLE_COUNT-1].closing = incrementForGraph(coin->current_price);
}

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
      opening_y = (candles[i].opening - min_val) * ((double) graph_top - (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
      closing_y = (candles[i].closing - min_val) * ((double) graph_top - (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
      high_y = (candles[i].high - min_val) * ((double) graph_top - (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
      low_y = (candles[i].low - min_val) * ((double) graph_top - (double) graph_bottom) / (max_val - min_val) + (double) graph_bottom;
    } else {
      opening_y = ((tft.height()/2) + (tft.height()-5))/2;
      closing_y = ((tft.height()/2) + (tft.height()-5))/2;
      high_y = ((tft.height()/2) + (tft.height()-5))/2;
      low_y = ((tft.height()/2) + (tft.height()-5))/2;
    }

    int bar_x = map(i, 0, CANDLE_COUNT, 2, tft.width()-2);
    uint16_t color; 

    if (candles[i].opening > candles[i].closing){
      color = RED;
    } else {
      color = ST77XX_GREEN;
    }

    tft.drawLine(bar_x, high_y, bar_x+2, high_y, color);
    tft.drawLine(bar_x+1, high_y, bar_x+1, min(opening_y, closing_y), color);
    tft.fillRect(bar_x, min(opening_y, closing_y), CANDLE_WIDTH, max(opening_y, closing_y) - min(opening_y, closing_y), color);
    tft.drawLine(bar_x+1, low_y, bar_x+1, max(opening_y, closing_y), color);
    tft.drawLine(bar_x, low_y, bar_x+2, low_y, color);
  }
}

void getData()
{
  // Instanciate Secure HTTP communication
  client.setFingerprint(fingerprint);
  
  http.begin(client, CG_URL);

  int httpCode = http.GET();

  if (httpCode > 0) {
      // Get the request response payload (Price data)
      payload = http.getString();
      StaticJsonDocument<800> doc;

      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        delay(5000);
        return;
      }

      // Parameters
      // The ArduinoJson assistant will generate your readable parameters.  Paste them here and remove any that you don't need.
//      JsonObject current;
//      for (int i = 0; i < COIN_COUNT; i++){
//          current = doc[coins[i].coin_id];
//          coins[i].current_price = current["gbp"];
//          coins[i].current_change = current["gbp_24h_change"];
//          addPrice(coins+i, incrementForGraph(current["gbp"])); 
//      }

      JsonObject current = doc["bitcoin"];
      coins[0].current_price = current["gbp"];
      coins[0].current_change = current["gbp_24h_change"];
      addPrice(coins, incrementForGraph(current["gbp"])); 

      current = doc["ethereum"];
      coins[1].current_price = current["gbp"];
      coins[1].current_change = current["gbp_24h_change"];
      addPrice(coins+1, incrementForGraph(current["gbp"])); 

      current = doc["cardano"];
      coins[2].current_price = current["gbp"];
      coins[2].current_change = current["gbp_24h_change"];
      addPrice(coins+2, incrementForGraph(current["gbp"])); 

      current = doc["algorand"];
      coins[3].current_price = current["gbp"];
      coins[3].current_change = current["gbp_24h_change"];
      addPrice(coins+3, incrementForGraph(current["gbp"])); 

      current = doc["chainlink"];
      coins[4].current_price = current["gbp"];
      coins[4].current_change = current["gbp_24h_change"];
      addPrice(coins+4, incrementForGraph(current["gbp"])); 

      current = doc["ripple"];
      coins[5].current_price = current["gbp"];
      coins[5].current_change = current["gbp_24h_change"];
      addPrice(coins+5, incrementForGraph(current["gbp"])); 

      current = doc["matic-network"];
      coins[6].current_price = current["gbp"];
      coins[6].current_change = current["gbp_24h_change"];
      addPrice(coins+6, incrementForGraph(current["gbp"])); 

      current = doc["tron"];
      coins[7].current_price = current["gbp"];
      coins[7].current_change = current["gbp_24h_change"];
      addPrice(coins+7, incrementForGraph(current["gbp"])); 

      current = doc["cosmos"];
      coins[8].current_price = current["gbp"];
      coins[8].current_change = current["gbp_24h_change"];
      addPrice(coins+8, incrementForGraph(current["gbp"])); 

      current = doc["binancecoin"];
      coins[9].current_price = current["gbp"];
      coins[9].current_change = current["gbp_24h_change"];
      addPrice(coins+9, incrementForGraph(current["gbp"])); 

      current = doc["tether"];
      coins[10].current_price = current["gbp"];
      coins[10].current_change = current["gbp_24h_change"];
      addPrice(coins+10, incrementForGraph(current["gbp"])); 

      current = doc["basic-attention-token"];
      coins[11].current_price = current["gbp"];
      coins[11].current_change = current["gbp_24h_change"];
      addPrice(coins+11, incrementForGraph(current["gbp"])); 
  }

  http.end();   //Close connection
}

COIN* createCoin(char* code, char* id, const unsigned char* bitmap, uint16_t circle_col, uint16_t bm_col){
  COIN* new_coin = (COIN*) malloc(sizeof(COIN));
  new_coin->coin_code = code;
  new_coin->coin_id = id;
  new_coin->bitmap = bitmap;
  new_coin->candles = (G_CANDLE*) malloc(sizeof(G_CANDLE)*CANDLE_COUNT);
  initialiseCandles(new_coin->candles);
  new_coin->circle_colour = circle_col;
  new_coin->bm_colour = bm_col;
  return new_coin;
}

G_CANDLE* createCandle(){
  G_CANDLE* candle = (G_CANDLE*) malloc(sizeof(G_CANDLE));
  candle -> opening = -1;
  candle -> closing = -1;
  candle -> low = -1;
  candle -> high = -1;

  return candle;
}

G_CANDLE initialiseCandles(G_CANDLE* candle_arr){
  for (int i = 0; i < CANDLE_COUNT; i++){
    G_CANDLE* new_candle = createCandle();
    candle_arr[i] = *new_candle;
  }
}

float incrementForGraph(double val){
  float to_incr = (float) val;
  while(to_incr < 10000){
    to_incr*=10;
  }
  return to_incr;
}

void drawName(String coin_name){
  tft.setTextSize(2);
  if (coin_name.length() == 3){
    tft.setCursor(NAME_START_3, 6);
  } else if (coin_name.length() == 4) {
    tft.setCursor(NAME_START_4, 6);
  } else {
    tft.setCursor(NAME_START_5, 6);
  }
  tft.print(coin_name);
}

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
  
  tft.print(print_price);
}

void drawPercentageChange(float change){
  tft.setCursor(CHANGE_START_X, CHANGE_START_Y);
  tft.setTextSize(1);
  tft.print("24h Change: ");
  if (change < 0){
    tft.setTextColor(RED);
  } else {
    tft.setTextColor(ST77XX_GREEN);
    tft.print('+');
  }
  tft.print(change);
  tft.setTextColor(WHITE);
}

void drawBitmap(int16_t x, int16_t y,const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
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

String getFormattedTimeNoSeconds(NTPClient timeClient) {
  unsigned long rawTime = timeClient.getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  return hoursStr + ":" + minuteStr;
}

void drawTime(){
  tft.fillRect(0, tft.height()-20, tft.width(), 20, BLACK);

  tft.setCursor(3, tft.height()-10);
  tft.setTextSize(1);

  tft.print(daysOfTheWeek[timeClient.getDay()]);
  tft.setCursor(tft.width()-32, tft.height()-10);
  tft.print(getFormattedTimeNoSeconds(timeClient));
}

void clearEEPROM(){
  // Clear EEPROM
  for (int i = 0; i < 512; i++){
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void printEEPROM(){
  // Clear EEPROM
  for (int i = 0; i < 512; i++){
    Serial.print(EEPROM.read(i));
    Serial.print(" ");
  }
}

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
