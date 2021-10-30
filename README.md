# ESP8266_ST7735_IR_CryptoTicker
ESP8266 NodeMCU powered Crypto Ticker that uses an ST7735 TFT screen to display statistics and the candle chart for various cryptocurrencies, also utilising an IR remote for configuration and user input. Also has functionality that allows the user to add their own portfolio.

## Initialisation/Network Config
When the device is plugged in, it displays the logo/name and gives the user the opportunity to clear any existing WiFi credentials from the EEPROM memory if needed (e.g. changed router SSID).

If the device detects that there are credentials stored in the EEPROM, it will use these to try and connect the the router with the given SSID, using the given password.

If the device detects that there are no network credentials stored in the EEPROM, then the user is presented with a keyboard that they can use to enter their SSID first, and then their password second. These details are then used to connect to the network.

## Crypto Display

This screen displays characteristics of coins selected by the user in the menu, it shows the current price, 24hr change, as well as a candle chart of the coin (which fills up over time). Up to 8 coins can be selected, and the coins are cycled through at a rate that can be selected by the user in the menu.

<img src="https://user-images.githubusercontent.com/47477832/137286323-0c987a22-59e3-42f3-9926-4633200aa912.jpg" width="350">
<img src="https://user-images.githubusercontent.com/47477832/137286402-810c95b3-79af-4720-b935-4e37dc9c8246.jpg" width="350">

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
