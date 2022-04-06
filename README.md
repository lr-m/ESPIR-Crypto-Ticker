# ESP8266_ST7735_IR_CryptoTicker
ESP8266 NodeMCU powered Crypto Ticker that uses an ST7735 TFT screen to display statistics and the candle chart for various cryptocurrencies, also utilising an IR remote for configuration and user input. Also has functionality that allows the user to add their own portfolio.

<img src="https://user-images.githubusercontent.com/47477832/161318551-55cef6e2-5b4a-4e39-bbb0-8781566d7513.png" width="500">

**NOTE: If you are using this and would like any more features or modifications to be made please let me know by opening an issue, with the title beginning as 'Feature Request - '.**

## IMPORTANT

To use, put the ST7735_Crypto_Ticker directory in your Arduino libraries directory. Also, you need to add the thumbprint from your browser to the Arduino sketch (line 59 of ticker.ino). In the event the display is not properly aligned (edges of pixels with random colour) or the colours are inverted, look at the Display Fix section.

To get the thumbprint (on Chrome):

1. Visit https://www.coingecko.com/api/documentations/v3#/
2. Click the lock next to the URL/Search Bar
<img src="https://user-images.githubusercontent.com/47477832/141087977-d66bf865-c321-4be8-a7fc-5842b174abb6.png" width="200">

3. In the menu, click 'Connection is Secure'
<img src="https://user-images.githubusercontent.com/47477832/141087990-272294f7-a46d-4fca-abb0-e3807b0af890.png" width="150">

4. Then click 'Certificate Is Valid'
<img src="https://user-images.githubusercontent.com/47477832/141088004-75575b7c-f048-4956-ba34-611ce81b0ee0.png" width="150">

5. Click on the 'details' tab that appears
6. Scroll down to the 'thumbprint' field, click this and copy the thumbprint
<img src="https://user-images.githubusercontent.com/47477832/141088030-66d95251-5cf8-4dd5-860b-816f506b50da.png" width="250">

7. Replace line 84 of ticker.ino with the copied fingerprint, for example:

```const char *fingerprint = "*YOUR FINGERPRINT HERE (TUTORIAL IN INSTRUCTIONS)";```

Becomes:

```const char *fingerprint = "4j3krnm34k53j6n4k3kj5n6kl3l54n5k4m532";```

(Invalid example fingerprint used above, do not copy)

### Display Fix

There are 2 types of ST7735 screens, black tab and green tab. Although if the screen has a green tab, is sometimes acts like a black tag in my experience so this classification isn't 100% correct. If the display is working, but is misaligned and the colours are inverted, you will need to swap which type of screen the sketch is configured with, this involves modification of 2 files:

- In the Arduino library, ST7735_Crypto_Ticker, the colours.h file needs to be edited to comment out the set colours that are not currently commented out, and to comment out the current set colours. 

- In the arduino sketch itself, comment out line 133, and uncomment line 134. This will configure the device with the black tab settings.

These modifications should fix the problem.

## Components
- NodeMCU ESP8266 WiFi Microcontroller CP2102
- IR Reciever / Remote
- ST7735 1.77" display (Will support different sizes when I can get some more displays to play with)

### Configuration
<img src="https://user-images.githubusercontent.com/47477832/140657043-b1d6ede4-3b15-4b09-be88-481e7d5b064b.png" width="500">

## Libraries
- In Arduino Boards Manager - ESP8266 Boards (2.7.4)
- IRremote v3.3.0
- Adafruit_GFX_library (for Adafruit_GFX.h)
- Adafruit_ST7735_and_ST7789_Library (for Adafruit_ST7735.h)

## Initialisation/Network Config
When the device is plugged in, it displays the logo/name and gives the user the opportunity to clear any existing WiFi credentials from the EEPROM memory if needed (e.g. changed router SSID).

If the device detects that there are credentials stored in the EEPROM, it will use these to try and connect the the router with the given SSID, using the given password.

If the device detects that there are no network credentials stored in the EEPROM, then the user is presented with a keyboard that they can use to enter their SSID first, and then their password second. These details are then used to connect to the network.

## Crypto Display

This screen displays characteristics of coins selected by the user in the menu, it shows the current price, 24hr change, as well as a candle chart of the coin (which fills up over time). Up to 8 coins can be selected, and the coins are cycled through at a rate that can be selected by the user in the menu.

<img src="https://user-images.githubusercontent.com/47477832/139539473-5c04ddb7-ac7f-4104-9e64-166611e64a84.jpg" width="350">
<img src="https://user-images.githubusercontent.com/47477832/139539466-6838b78b-e29e-4d7e-8318-47401f1934f9.jpg" width="350">

### Controller Navigation
- '#' - This opens the Crypto menu that allows configuration and coin selection.
- '&#8594;' - Moves the display along to the next coin.
- '&#8592;' - Moves the display to the previous coin.
- '&#8593;' - Moves to the next screen (Portfolio/Coin).
- '&#8595;' - Moves to the previous screen (Portfolio/Coin).

## Crypto Menu

This is where the user can configure some of the aspects of the crypto interface display. The options that can be configured are:
- Coin list - What coins are being updated and displayed.
- Add new coin - Here is where users can add any coin that is available on CoinGecko
- Candle delay - How long the time frame of each candle in the chart is.
- Coin cycle delay - The duration between automatic cycling of coins.

<img src="https://user-images.githubusercontent.com/47477832/138352726-f378f234-b823-466a-8eec-8e7c0f87b6b7.jpg" width="350">

### Controller Navigation

- '&#8593;' - Selects the menu item above the currently selected item.
- '&#8595;' - Selects the menu item below the currently selected item.

## Add New Coin

To add a new coin, you need to know a few things, the api ID, the code, and the rgb values of the colour you want to represent it.

#### Step 1 - Select coin to replace
- Need to replace a coin in the existing coins with the new coin, so select a coin to replace (navigating with arrows and selecting with 'OK').

<img src="https://user-images.githubusercontent.com/47477832/139539178-94739a21-270e-48e3-8fa8-addb168e3221.jpg" width="350">

#### Step 2 - Enter ID and Code of token
- To find the ID of the token, find it on CoinGecko and look for the API key, this is what you need. For example, for the coin Monero (XMR), you would navigate to https://www.coingecko.com/en/coins/monero to find the api ID, which in this case is 'monero'. The code can also be found here. Use the keyboard to enter the values and hit enter when completed.
- There is a check in place that ensures the provided id is valid.

<img src="https://user-images.githubusercontent.com/47477832/139539189-ba87a33e-c34f-428f-9506-14d2eeea6cf2.jpg" width="350">

#### Step 3 - Enter the colour you want to represent the coin
- Need to select a colour to represent the coin in the portfolio and coin interfaces, using the rgb value found earlier, enter these into their respective entry point using the up and down arrows to adjust them (+-10 each time). Then hit enter when this is completed.

<img src="https://user-images.githubusercontent.com/47477832/139539200-7f4f052d-43b2-41fb-bf82-f24453523ad7.jpg" width="350">

#### Now the coin has been added

<img src="https://user-images.githubusercontent.com/47477832/139539217-48aa2375-cc52-4f42-af42-204172f71bde.jpg" width="350">

## Portfolio Example (Random Coins and Amounts)

On this screen, properties of the users portfolio can be displayed. The user has to configure their portfolio before they can see proportions using the portfolio editor found in the menu.

### Section 1 - Coin Details

This section displays properties of the coins that make up the users portfolio, such as name, current price, and 24 hour change.

<img src="https://user-images.githubusercontent.com/47477832/137373359-00ccda17-8f93-40ca-89bd-db0e0d09f09f.jpg" width="350">

### Section 2 - Portfolio Proportions

This section displays the propertions of your portfolio, both graphically as a pie chart, and also a simple list of the percentage of the portfolio each coin is.

<img src="https://user-images.githubusercontent.com/47477832/137373354-35c3d439-3129-4abf-86ca-88a0efb7dda7.jpg" width="350">

### Section 3 - Portfolio Candles

This section displays the candle graph of the portfolio, similarly to the coin interface it takes time to fill up. Also displays the proportional bar seen in the first section.

<img src="https://user-images.githubusercontent.com/47477832/138352170-c8b0ba11-bc68-4fd0-9fd1-a115281a0272.jpg" width="350">

### Controller Navigation

- '#' - Opens the menu.
- '&#8594;' - Moves to the next section display.
- '&#8592;' - Moves to the previous section display.
- '&#8593;' - Moves to the next screen (Portfolio/Coin).
- '&#8595;' - Moves to the previous screen (Portfolio/Coin).

## Portfolio Menu

This is where the user can configure the settings of the Portfolio screen, and also access the Portfolio Editor.

<img src="https://user-images.githubusercontent.com/47477832/138352360-5580b8b8-b8d9-427f-88a4-bff018310c92.jpg" width="350">

### Controller Navigation
- '&#8593;' - Selects the menu item above the currently selected item.
- '&#8595;' - Selects the menu item below the currently selected item.

## Portfolio Editor

This is where the user can configure their Portfolio, selecting amounts of coins that they own.

<img src="https://user-images.githubusercontent.com/47477832/137286628-f72da208-2558-4e68-b25a-122f7477f625.jpg" width="350">

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
