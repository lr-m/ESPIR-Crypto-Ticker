# ESPIR Crypto
ESP8266 NodeMCU powered Crypto Ticker that uses an ST7735 TFT screen to display statistics and the candle chart for various cryptocurrencies, also utilising an IR remote for configuration and user input. Also has functionality that allows the user to add their own portfolio. This device is powered by the CoinGecko API.

<img src="https://user-images.githubusercontent.com/47477832/174843170-61d2b933-98b9-4687-ba08-1a4094de4f7b.jpg" width="500">

**NOTE: If you are using this and would like any more features or modifications to be made please let me know by opening an issue, with the title beginning with 'Feature Request'.**

## IMPORTANT

- To use, put the ST7735_Crypto_Ticker directory in your Arduino libraries directory, dependencies will also need to be installed. 
- If the display is not properly aligned (edges of pixels with random colour) or the colours are inverted, look at the Display Fix section.
- If you have used a previous version of this software then you may have issues with the EEPROM storage, to resolve any issues, press '#' during the initial boot screen (with the logo) to completely clear the EEPROM, you will need to re-enter your WiFi credentials but everything will work fine after.

### Display Fix

There are 2 types of ST7735 screens, black tab and green tab. Although if the screen has a green tab, is sometimes acts like a black tag in my experience so this classification isn't 100% correct. If the display is working, but is misaligned and the colours are inverted, you will need to swap which type of screen the sketch is configured with, this involves modification of 2 files:

- In the Arduino library, ST7735_Crypto_Ticker, the colours.h file needs to be edited to comment out the set colours that are not currently commented out, and to comment out the current set colours. 

- In the arduino sketch itself, comment out line 158 (//), and uncomment line 157. This will configure the device with the green tab settings.

These modifications should fix the problem, if there are still issues, try and modify only 1 of them.

## Libraries
- In Arduino Boards Manager - ESP8266 Boards (2.7.4)
- IRremote v3.3.0
- Adafruit_GFX_library (for Adafruit_GFX.h)
- Adafruit_ST7735_and_ST7789_Library (for Adafruit_ST7735.h)

## Components
- NodeMCU ESP8266 WiFi Microcontroller CP2102
- IR Reciever / Remote
- ST7735 1.77" display

### Configuration
<img src="https://user-images.githubusercontent.com/47477832/164561761-e92a895c-4aa3-46f4-a08d-b6ffae2523e9.png" width="500">

## Initialisation/Network Config
When the device is plugged in, it displays the logo/name and gives the user the opportunity to clear any existing WiFi credentials from the EEPROM memory if needed (e.g. changed router SSID).

If the device detects that there are credentials stored in the EEPROM, it will use these to try and connect the the router with the given SSID, using the given password.

If the device detects that there are no network credentials stored in the EEPROM, then the user is presented with a keyboard that they can use to enter their SSID first, and then their password second. These details are then used to connect to the network.

## Crypto Display

This screen displays characteristics of coins selected by the user in the menu, it shows the current price, 24hr change, as well as a candle chart of the coin (which fills up over time). Up to 8 coins can be selected, and the coins are cycled through at a rate that can be selected by the user in the menu.

<img src="https://user-images.githubusercontent.com/47477832/174843491-02c85839-9401-4210-b025-e5a2e95687bc.jpg" width="350">

<img src="https://user-images.githubusercontent.com/47477832/175394447-7b83730b-44e7-455b-a5e8-592797f1f080.jpg" width="350">

### Controller Navigation
- '#' - This opens the Crypto menu that allows configuration and coin selection.
- '&#8594;' - Moves the display along to the next coin.
- '&#8592;' - Moves the display to the previous coin.
- '&#8593;' - Moves to the next screen (Portfolio/Coin).
- '&#8595;' - Moves to the previous screen (Portfolio/Coin).

## Crypto Menu

<img src="https://user-images.githubusercontent.com/47477832/174843700-1f6f471d-a297-47a6-be3a-393fe8167a3e.jpg" width="350">

This is where the user can configure some of the aspects of the crypto interface display. The options that can be configured are:
- Edit coin list - What coins are being updated and displayed.
- Add new coin - Here is where users can add any coin that is available on CoinGecko
- Settings - Sub menu where user can change properties of the coin element of the device
- Clear WiFi credentials - This simply removes the saved WiFi credentials for the device EEPROM memory
- Reset coins (restart) - This clears any added coins and restores the initial bitmaps and states for all coins (will cause a restart of the device)

### Coin Display Settings

<img src="https://user-images.githubusercontent.com/47477832/175394585-607c79a0-c6bc-47cb-89d7-eb4cfada27e0.jpg" width="350">

- Candle delay - How long the time frame of each candle in the chart is.
- Coin cycle delay - The duration between automatic cycling of coins.
- Currency - USD/GBP/EUR
- Toggle bitmaps - Enables/disables the bitmaps in the top left corner of the display for all coins

### Controller Navigation

- '&#8593;' - Selects the menu item above the currently selected item.
- '&#8595;' - Selects the menu item below the currently selected item.

## Add New Coin

To add a new coin, you need to know a few things, the api ID, the code, and the rgb values of the colour you want to represent it.

#### Step 1 - Select coin to replace
- Need to replace a coin in the existing coins with the new coin, so select a coin to replace (navigating with arrows and selecting with 'OK').

<img src="https://user-images.githubusercontent.com/47477832/174847262-a57a7ee5-2ea3-4f97-b95c-472557f5414f.jpg" width="350">

#### Step 2 - Enter ID and Code of token
- To find the ID of the token, find it on CoinGecko and look for the API key, this is what you need. For example, for the coin Monero (XMR), you would navigate to https://www.coingecko.com/en/coins/monero to find the api ID, which in this case is 'monero'. The code can also be found here. Use the keyboard to enter the values and hit enter when completed.
- There is a check in place that ensures the provided id is valid.

<img src="https://user-images.githubusercontent.com/47477832/174847283-075955b1-b701-48e7-9e15-9c3f62bd7301.jpg" width="500">

#### Step 3 - Enter the colour you want to represent the coin
- Need to select a colour to represent the coin in the portfolio and coin interfaces, using the rgb value found earlier, enter these into their respective entry point using the up and down arrows to adjust them (+-10 each time). Then hit enter when this is completed.

<img src="https://user-images.githubusercontent.com/47477832/139539200-7f4f052d-43b2-41fb-bf82-f24453523ad7.jpg" width="350">

#### Now the coin has been added

<img src="https://user-images.githubusercontent.com/47477832/174847318-a65be34d-1488-4f36-804f-d08e9ae60a30.jpg" width="350">

## Portfolio Example (Random Coins and Amounts)

On this screen, properties of the users portfolio can be displayed. The user has to configure their portfolio before they can see proportions using the portfolio editor found in the menu.

### Section 1 - Coin Details

This section displays properties of the coins that make up the users portfolio, such as name, current price, and 24 hour change.

<img src="https://user-images.githubusercontent.com/47477832/174843891-8b0a30cd-67ef-4d7f-8005-7ae4dcf17c91.jpg" width="350">

### Section 2 - Portfolio Proportions

This section displays the propertions of your portfolio, both graphically as a ring/pie chart, also a simple list of the percentage of the portfolio each coin is.

<img src="https://user-images.githubusercontent.com/47477832/175395894-3cbcee95-3976-483b-9391-0a1856508b51.jpg" width="350">

### Section 3 - Portfolio Candles

This section displays the candle graph of the portfolio, similarly to the coin interface it takes time to fill up. Also displays the proportional bar seen in the first section.

<img src="https://user-images.githubusercontent.com/47477832/174843930-71b3227b-bacc-47ee-a94d-cbf76ac2c978.jpg" width="350">

### Controller Navigation

- '#' - Opens the menu.
- '&#8594;' - Moves to the next section display.
- '&#8592;' - Moves to the previous section display.
- '&#8593;' - Moves to the next screen (Portfolio/Coin).
- '&#8595;' - Moves to the previous screen (Portfolio/Coin).

## Portfolio Menu

This is where the user can configure the settings of the Portfolio screen, and also access the Portfolio Editor.

<img src="https://user-images.githubusercontent.com/47477832/174844074-63426e36-a9ba-497f-99cd-5ef2ec519f03.jpg" width="350">

### Controller Navigation
- '&#8593;' - Selects the menu item above the currently selected item.
- '&#8595;' - Selects the menu item below the currently selected item.

## Portfolio Editor

This is where the user can configure their Portfolio, selecting amounts of coins that they own.

<img src="https://user-images.githubusercontent.com/47477832/174848638-8013ba08-5941-4e55-90f7-51cbc87c75d4.jpg" width="350">

### Controller Navigation
#### Coin Not Selected
- '&#8594;' - Moves to the next coin to select.
- '&#8592;' - Moves to the previous coin to select.
- '#' - Go back to Portfolio Menu.
- 'OK' - Enter the amount changer for the currently selected coin.

#### Coin Selected
- '&#8594;' - Multiplies the amount to add by 10.
- '&#8592;' - Divides the amount to add by 10.
- '&#8593;' - Adds the current amount to add to the coin amount.
- '&#8595;' - Subtracts the current amount to add to the coin amount.
- '#' - Go back to Portfolio Menu.
- 'OK' - Exit the amount changer for the currently selected coin and return to coin selection.

## EEPROM Properties

### WiFi credentials
- WiFi credentials are stored and displayed in *plaintext*, make sure you clear the EEPROM if you are not going to use the device anymore
- WiFi credentials are also displayed in *plaintext* during network initialisation (makes things easier to debug), so also be aware of that

### Settings
- There can be up to 9 selected coins at any given time, these are stored in the EEPROM and are loaded back during initialisation
- There are flags that indicate what settings have been selected in the menus

### Added coins
- Up to 5 coins can be manually added (low number due to memory constraints)
- Once 5 have been added, any extra will overwrite the first added coin

### Portfolio
- Amounts in portfolio are stored, up to 9 coin amounts can be stored and reloaded (this is the maximum that can be displayed on the screen)
- Can be cleared with the menu option
