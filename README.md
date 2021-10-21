# ESP8266_ST7735_IR_CryptoTicker
ESP8266 NodeMCU powered Crypto Ticker that uses an ST7735 TFT screen to display statistics and the candle chart for various cryptocurrencies, also utilising an IR remote for configuration and user input. Also has functionality that allows the user to add their own portfolio.

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
- Candle delay - How long the time frame of each candle in the chart is.
- Coin cycle delay - The duration between automatic cycling of coins.

<img src="https://user-images.githubusercontent.com/47477832/138352726-f378f234-b823-466a-8eec-8e7c0f87b6b7.jpg" width="350">

### Controller Navigation

- '&#8593;' - Selects the menu item above the currently selected item.
- '&#8595;' - Selects the menu item below the currently selected item.

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
