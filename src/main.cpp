/**
 * @file main.cpp
 * @remarks uSonicTimer
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief an addition to an old, inexpensive Ultrasonic cleaner to include heating and timing
 * @version 0.4
 * @date 08/28/24
 *
 * @copyright Copyright (c) 2024 Somerled Design, LLC in Kevin Murphy
 *
 *                      GNU GENERAL PUBLIC LICENSE
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *   This code is currently maintained for and compiled with Arduino 1.8.x.
 *   Your mileage may vary with other versions.
 *
 *   ATTENTION: LIBRARY FILES MUST BE PUT IN LIBRARIES DIRECTORIES AND NOT THE INO SKETCH DIRECTORY !!!!
 *
 *   FOR EXAMPLE:
 *   tiny4kOLED.h, tiny4kOLED.cpp ------------------------>   \Arduino\Sketchbook\libraries\tiny4kOLED\
 *
 *   Version History -
 *
 *   .100 - 01/16/22 - A work in progress
 *   .200 - 01/29/22 - rewrite with u8g2lib as display driver
 *   .300 - 08/20/24 - rewrite by GROK2.mini
 *   .400 - 08/28/24 - rewrite of Grok2 code by Kevin Murphy
 *
 * DISCLAIMER:
 *   With this design, including both the hardware & software I offer no guarantee that it is bug
 *   free or reliable. So, if you decide to build one and you have problems or issues and/or causes
 *   damage/harm to you, others or property then you are on your own. This work is experimental.
 *
 */
/**
 *  === ESP8266 Pinout ===
 *  --- Available  I/O ---
 *   •   GPIO0     D3
 *   •   GPIO1     D10 (TX) \
 *   •   GPIO3     D9  (RX)  |  If using as inputs don't call Serial.begin()
 *
 *   •   GPIO2     D4  (SCL)   \
 *                               - I2C
 *   •   GPIO4     D2  (SDA)   /
 *
 *   •   GPIO5     D1
 * 
 *   ◎   GPIO6     ☞   USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO7     ☞   USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO8     ☞   USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO9     ☞   USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO10    ☞   USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO11    ☞   USED INTERNALLY - DO NOT USE **
 * 
 *   ●   GPIO12    D6  (MISO) | \
 *   •   GPIO13    D7  (MOSI) |    S
 *   •   GPIO14    D5  (SCLK) |     P
 *   •   GPIO15    D8  (CS)   | /    I
 *   •   GPIO16    D0  (no interrupt)
 *   •   ADC0      A0  (Analog Input)
 *
 * ----------------------------------------------------------------------------------------
 *  Required I/O (PCB v1b)
 *    SPI Nokia 5110
 *      Func          Non i/o     Digitial     GPIO i/o     ESP12e Pin         Descr.
 *     =====================================================================================
 *    1 RESET          RST        RST          -           1                  RESET line
 *    2 CS             GND        -            -           -                  CHIP SELECT (tied low)
 *    3 D/C                       D10/TX       GPIO1       22                 DATA|COMMAND
 *    4 DIN                       D7(MOSI)     GPIO13      7                  MOSI
 *    5 CLK                       D5(SCLK)     GPIO14      5                  SCLK
 *    6 VCC            3V
 *    7 BL                        D4           GPIO2       17                 Backlight PWM
 *    8 GND
 *
 *    Rotary Encoder
 *      Func          Non i/o     Digitial     GPIO i/o     ESP12e Pin        Descr.
 *     =====================================================================================
 *      ROT_ENC_A_PIN             D3           GPIO0       18                 CLK/~PROGRAM
 *      ROT_ENC_B_PIN             D6           GPIO12      6                  DT
 *      ROT_ENC_BUTTON_PIN        D9           GPIO3       21                 SW/RX
 *
 *    Relays
 *      Func          Non i/o     Digitial     GPIO i/o     ESP12e Pin         Descr.
 *     =====================================================================================
 *      HEATER_ENABLE_PIN         D2           GPIO4       19                 Heater relay
 *      DEVICE_ENABLE_PIN         D1           GPIO5       20                 Cleaner relay
 *
 *    Dallas Temp Sensor (DS18B20)
 *      Func          Non i/o     Digitial     GPIO i/o     ESP12e Pin         Descr.
 *     =====================================================================================
 *      ONE_WIRE_BUS              D0           GPIO16       4
 */
/**
 *
    Notes:
    This code assumes you have the necessary libraries installed. For ESPRotary and Button2, you might need to install them manually or via the Arduino IDE's library manager.
    The networkSettings() function is left unimplemented as it would require specific details about how you want to handle network connections.
    The saveSettings() and loadSettings() functions use EEPROM to persist settings across power cycles. Ensure you have enough EEPROM space on your ESP8266 module.
    The code uses a simple state machine for menu navigation, which should be expanded for more complex interactions or additional menu items.
    Error handling, especially for temperature sensor readings or network operations, should be added for robustness.
    Adjust the pin numbers according to your actual hardware setup.
 */
/**
 * GROK2:
 * Below is a C++ Arduino program tailored for an ESP8266, integrating the
 * functionalities you've described for a menu-driven countdown timer system
 * with temperature control and ultrasonic cleaner operation. This program uses
 * the libraries you've specified and includes additional functionality for
 * network settings and display adjustments.
 *
 */
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESPRotary.h>
#include <Button2.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include "Ticker.h" // https://github.com/esp8266/Arduino/tree/master/libraries/Ticker

#ifdef WITH_GDB
#include "GDBStub.h"
#endif

#if DEBUG
#define debugbegin(x) Serial.begin(x)
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debugbegin(x)
#define debug(x)
#define debugln(x)
#endif // DEBUG

// Pin definitions (PCB v1b)
#define ONE_WIRE_BUS D0 // GPIO16

#define CLEANER_PIN D1  // GPIO5
#define HEATER_PIN D2   // GPIO4

#define ROTARY_PIN1 D3     // GPIO0, CLK/~PROGRAM
#define ROTARY_PIN2 D6     // GPIO12, DT
#define ROTARY_BUTTON D9   // GPIO3, SW/RX

#define BACKLIGHT_PIN D4 // GPIO2

#define LCD_SCLK_PIN D5   // GPIO14
#define LCD_DIN_PIN D7    // GPIO13
#define LCD_DC_PIN D10    // GPIO1 (TX)
#define LCD_CS_PIN U8X8_PIN_NONE
#define LCD_RST_PIN U8X8_PIN_NONE
// Status LED
// TODO: maybe use the backlight as a Status?  Pulsing, Flashing, Steady, Dim?
// #define STATUS_LED_PIN ?
// bool statusLedOn = false;

#define CLICKS_PER_STEP 4
// -------------------------------------------------------------------------
//  NOKIA 5110 LCD
// #define sclk_pin D5
// #define dc_pin D6
// #define din_pin D7
// #define cs_pin D8
// #define rst_pin -1 // as in the Adafruit header...
//                       physically conn to RST switch
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(
    U8G2_R0,        /* Rotation 0 = no rotation */
    LCD_SCLK_PIN,   /* clock */
    LCD_DIN_PIN,    /* data */
    LCD_CS_PIN,     /* cs */
    LCD_DC_PIN,     /* dc */
    LCD_RST_PIN     /* reset */
);

// Temp Sensor Data Bus
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// TODO: is this the correct address?
//  address of thermometer on oneWire bus
//  are they different for each DS18B20?
//                                  0x28, 0xFF, 0x57, 0x3F, 0x01, 0x16, 0x01, 0xED
DeviceAddress thermometerAddress; // custom array type to hold 64 bit device address

// Rotary Encoder and button
ESPRotary r;
Ticker t;
Button2 b;

// Variables
float currentTemperature = 0;
uint8_t g_setTemperatureF;   // The set temperature in Fahrenheit
uint8_t g_timerSetting;      // The timer to be set in minutes
uint8_t tempOffset = 10;     // Offset in Fahrenheit for heater control
uint8_t g_contrast;          // The contrast for the display
uint16_t longPress = 1000;   // one second long press of button
int16_t last = 0;            // For rotary encoder reading

volatile bool down  = false; // Flags for the encoder
volatile bool up    = false;

// State variables
volatile bool cleanerOn = false; // State of the cleaner
volatile bool heaterOn = false;  // State of the heater

// Menu structure
enum MenuItems
{
    START_TIMER,
    SET_TIMER,
    SET_TEMP,
    NETWORK,
    CONTRAST,
    MENU_ITEMS_COUNT
};
uint8_t g_currentMenu = START_TIMER;

// Function definitions
void startTimerPage();
void setTimerSubmenu();
void setTemperatureSubmenu();
void networkSettings();
void adjustContrast();
void displayMenu();
void saveSettings();
void loadSettings();
void handleLoop();
void readRotaryEncoder();
void turnOnHeater();
void turnOffHeater();
void turnOnCleaner();
void turnOffCleaner();
void turnOnBacklight();
void turnOffBacklight();

void setup()
{
    debugbegin(115200);
    debug("Entered setup()...");

    // Initialize EEPROM for saving settings
    ///////////////////////////////////////////////////////////////
    EEPROM.begin(512);

    // delay(4000);

    loadSettings(); // Load settings from EEPROM

    // Initialize display
    ///////////////////////////////////////////////////////////////
    u8g2.begin();
    u8g2.setContrast(g_contrast);

    digitalWrite(BACKLIGHT_PIN, LOW); // turn off the backlight

    // Initialize temperature sensor
    ///////////////////////////////////////////////////////////////
    sensors.begin();

    // Initialize rotary encoder
    ///////////////////////////////////////////////////////////////
    r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
    last = r.getPosition();
    // encoder.setChangedHandler(rotate);
    // encoder.setLeftRotationHandler(rotate);
    // encoder.setRightRotationHandler(rotate);|

    // set different limits based on menu item
    // for the secondary screen being shown
    // encoder.setLowerOverflowHandler(lower);
    // encoder.setUpperOverflowHandler(upper);

    // Initialize button
    ///////////////////////////////////////////////////////////////
    b.begin(ROTARY_BUTTON);
    // set click handler callback used in the main menu
    // button.setClickHandler(buttonClicked);
    // button.setLongClickHandler(buttonLongPress);         // will only be called after the button has been released.

    // Initialize ticker
    ///////////////////////////////////////////////////////////////
    t.attach_ms(10, handleLoop); // Call handleLoop every 10ms

    // Initialize pins
    ///////////////////////////////////////////////////////////////

    pinMode(BACKLIGHT_PIN, OUTPUT);
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(CLEANER_PIN, OUTPUT);
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(CLEANER_PIN, LOW);

    // Read initial temperature
    sensors.requestTemperatures();
    currentTemperature = sensors.getTempFByIndex(0); // TODO: get address of sensor and use that instead

    // TODO: setup wifi
    // create a secret.h file as in nightdriver by Dave Plummer

    // TODO: setup ntp
    // incorporate easy NTP TZ DST.cpp

    // TODO: setup rtc
    // incorporate easy NTP TZ DST.cpp

    debugln("...setup Complete.");
}

void loop()
{

    debugln("Entering loop()...");

    displayMenu();

    // debug("displayMenu complete...");

    readRotaryEncoder();

    // debug("rotaryEncoder has been read...");

    turnOnBacklight();

    // handle encoder events on various menuItems
    ////////////////////////////////////////////
    if (down)
    {
        debug("down rotate...");

        down = false;
        g_currentMenu = (g_currentMenu + 1) % MENU_ITEMS_COUNT;
    } // wrap around to first menu item when going down
    
    if (up)
    {
        debug("up rotate...");
        up = false;
        g_currentMenu = (g_currentMenu + MENU_ITEMS_COUNT - 1) % MENU_ITEMS_COUNT;
    } // wrap around to last menu item when going up

    // handle button events on various menuItems
    ////////////////////////////////////////////

    if (b.wasPressed())
    {
        if (b.wasPressedFor() > longPress && g_currentMenu == START_TIMER)
        {

            debug("b.waspressfor longpress on START_TIMER menuitem...");

            startTimerPage();
        }
        else if (b.wasPressedFor() > longPress && g_currentMenu != START_TIMER)
        {
            // do nothing
            // No...wait. Do this - If backlight is off, turn it on.

            debugln("b.wasPressedFor shortpress and NOT on START_TIMER menuitem");
            if (digitalRead(BACKLIGHT_PIN) == LOW)
            {
                digitalWrite(BACKLIGHT_PIN, HIGH);
            }
            // and if backlight is on, turn it off.
            else
            {
                digitalWrite(BACKLIGHT_PIN, LOW);
            }
        }

        // click press
        // Switch on the selected menu item

        switch (g_currentMenu)
        {
        case START_TIMER:
            // wasn't a long press so do nothing
            break;
        case SET_TIMER:
            setTimerSubmenu();
            break;
        case SET_TEMP:
            setTemperatureSubmenu();
            break;
        case NETWORK:
            networkSettings();
            break;
        case CONTRAST:
            adjustContrast();
            break;
        default:
            // do nothing (how did we get here?)
            break;
        }
        debugln("Loop complete.");
    }
}

/**
 * @brief Shows the timer page
 *
 * This function is called when the user selects the "Start Timer"
 * menu item. It will display the timer counting down and
 * control the heater and ultrasonic cleaner. The user can
 * exit the timer page by pressing the button.
 */
void startTimerPage()
{
    unsigned long startTime = millis();
    while (true)
    {
        handleLoop();
        unsigned long currentTime = millis();
        int32_t remainingTime = static_cast<int32_t>(g_timerSetting) * 60 -
                                static_cast<int32_t>((currentTime - startTime) / 1000);

        if (remainingTime <= 0)
        {
            turnOffCleaner();
            turnOffHeater();
            break;
        }

        sensors.requestTemperatures();
        currentTemperature = sensors.getTempFByIndex(0);

        if (currentTemperature < (g_setTemperatureF - tempOffset))
        {
            turnOnHeater();
            turnOffCleaner();
        }
        else
        {
            turnOffHeater();
            turnOnCleaner();
        }

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(0, 10);
        u8g2.print("Time Left: ");
        u8g2.print(remainingTime / 60);
        u8g2.print(":");
        if ((remainingTime % 60) < 10)
        {
            u8g2.print("0");
        }
        u8g2.print(remainingTime % 60);
        u8g2.setCursor(0, 30);
        u8g2.print("Temp: ");
        u8g2.print(currentTemperature);
        u8g2.print("F / ");
        u8g2.print(g_setTemperatureF);
        u8g2.print("F");
        u8g2.sendBuffer();

        if (b.wasPressedFor() > longPress)
        {
            break;
        }
    }
    turnOffCleaner();
    turnOffHeater();
}

/**
 * @brief Shows the timer selection submenu
 *
 * The user can cycle through the available preset times (5, 10, 15, 20, 25, 30 minutes)
 * by rotating the encoder. The selected time is displayed on the screen.
 * The user can confirm the selection by pressing the encoder button, which will
 * save the new setting and exit the menu.
 */
void setTimerSubmenu()
{
    static const uint8_t presets[] = {3, 8, 10, 15, 20, 30, 60};
    const uint8_t presetsCount = sizeof(presets) / sizeof(presets[0]);
    uint8_t selectedIndex = 0;
    for (uint8_t i = 0; i < presetsCount; i++)
    {
        if (presets[i] == g_timerSetting)
        {
            selectedIndex = i;
            break;
        }
    }
    while (true)
    {
        handleLoop();
        readRotaryEncoder();
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(0, 10);
        u8g2.print("Set Timer: ");
        u8g2.print(presets[selectedIndex]);
        u8g2.print(" min");
        u8g2.sendBuffer();

        if (down)
        {
            down = false;
            selectedIndex = (selectedIndex + 1) % presetsCount;
        }
        else if (up)
        {
            up = false;
            selectedIndex = (selectedIndex + presetsCount - 1) % presetsCount;
        }

        if (b.wasPressed())
        {
            g_timerSetting = presets[selectedIndex];
            saveSettings();
            break;
        }
    }
}

/**
 * @brief Shows the temperature selection submenu
 *
 * The user can cycle through the three digits of the temperature by rotating the encoder.
 * The selected digit is highlighted on the screen.
 * The user can confirm the selection by pressing the encoder button, which will
 * save the new setting and exit the menu.
 * Holding the encoder button down for more than 1 second will also save and exit.
 */
void setTemperatureSubmenu()
{
    uint8_t cursorPosition = 0;
    /*
     * This line of code is initializing an array  of 3 unsigned 8-bit
     * integers (uint8_t digits[3]) with the individual digits of the
     * temperature value stored in g_setTemperatureF.
     *
     * Here's a breakdown of how it's done:
     *
     * g_setTemperatureF % 10 gets the last digit (ones place)of the
     * temperature value.
     * (g_setTemperatureF / 10) % 10 gets the middle digit (tens place)
     * of the temperature value. The division by 10 shifts the digits
     * one place to the right, and then the modulo 10 operation gets
     * the last digit of the result, which is the original tens place.
     * g_setTemperatureF / 100 gets the first digit (hundreds place)
     * of the temperature value. The division by 100 shifts the digits
     * two places to the right.
     * For example, if g_setTemperatureF is 123, the array digits would
     * be initialized with the values {3, 2, 1}.
     *
     */

    uint8_t digits[3] = {
        static_cast<uint8_t>(g_setTemperatureF / 100),
        static_cast<uint8_t>((g_setTemperatureF / 10) % 10),
        static_cast<uint8_t>(g_setTemperatureF % 10)};

    while (true)
    {
        handleLoop();
        readRotaryEncoder();
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(0, 10);
        u8g2.print("Set Temp: ");
        for (uint8_t i = 0; i < 3; i++)
        {
            if (i == cursorPosition)
            {
                u8g2.setDrawColor(1);
                u8g2.drawBox((i + 1) * 10, 10, 10, 10);
                u8g2.setDrawColor(0);
            }
            u8g2.print(digits[i]);
        }
        u8g2.print("F");
        u8g2.sendBuffer();

        if (down)
        {
            down = false;
            digits[cursorPosition] = (digits[cursorPosition] + 1) % 10;
        }
        else if (up)
        {
            up = false;
            digits[cursorPosition] = (digits[cursorPosition] - 1 + 10) % 10;
        }

        // move the cursor position
        if (b.wasPressed())
        {
            cursorPosition = (cursorPosition + 1) % 3;
        }

        // long press will save and exit
        if (b.wasPressedFor() > longPress)
        {
            g_setTemperatureF = digits[0] * 100 + digits[1] * 10 + digits[2];
            saveSettings();
            break;
        }
    }
}

/**
 * @brief Implementation for network settings
 *
 * This function is called when the network menu option is selected.
 * It should handle the network settings.
 *
 * @todo Implement network settings
 */
void networkSettings()
{
    // Implementation for network settings
    while (true)
    {
        handleLoop();
        if (b.wasPressed())
        {
            saveSettings();
            break;
        }
    }
}

/**
 * @brief Adjusts the display contrast
 *
 * Allows the user to cycle through contrast settings with the rotary encoder.
 * The selected contrast is highlighted on the display.
 * When the user presses the button, the current contrast is saved and the menu exits.
 */
void adjustContrast()
{
    while (true)
    {
        handleLoop();
        readRotaryEncoder();
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(0, 10);
        u8g2.print("Contrast: ");
        u8g2.print(g_contrast);
        u8g2.sendBuffer();

        if (up)
        {
            up = false;
            g_contrast = min(g_contrast + 1, 255);
        }
        else if (down)
        {
            down = false;
            g_contrast = max(g_contrast - 1, 0);
        }
        u8g2.setContrast(g_contrast);

        // long press will save and exit
        if (b.wasPressedFor() > longPress)
        {
            saveSettings();
            break;
        }
    }
}

/**
 * @brief Displays the main menu
 *
 * Clears the display, sets the font to u8g2_font_6x10_tf, and draws the menu items
 * on the display. The currently selected item is highlighted with a white box.
 */
void displayMenu()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    for (uint8_t i = 0; i < MENU_ITEMS_COUNT; i++)
    {
        u8g2.setCursor(0, (i + 1) * 10);
        if (i == g_currentMenu)
        {
            u8g2.setDrawColor(1);
            u8g2.drawBox(0, i * 10, 84, 10);
            u8g2.setDrawColor(0);
        }
        else
        {
            u8g2.setDrawColor(1);
        }
        switch (i)
        {
        case START_TIMER:
            u8g2.print("Start Timer");
            break;
        case SET_TIMER:
            u8g2.print("Set Timer");
            break;
        case SET_TEMP:
            u8g2.print("Set Temp");
            break;
        case NETWORK:
            u8g2.print("Network");
            break;
        case CONTRAST:
            u8g2.print("Contrast");
            break;
        }
    }
    u8g2.sendBuffer();
}

/**
 * @brief Handles the loop tasks for the rotary encoder and button.
 *
 * Call this method repeatedly to handle the rotary encoder and button events.
 */
void handleLoop()
{
    r.loop();
    b.loop();
}

void readRotaryEncoder()
{
    int16_t position = r.getPosition();

    if (position > last)
    {
        last = position;
        down = true;
    }
    else if (position < last)
    {
        last = position;
        up = true;
    }
}

// =================================================================
// =================================================================
// Device control functions
// -----------------------------
/**
 * @brief Turns on the heater
 *
 * This function will turn on the heater by setting the HEATER_PIN
 * high and setting the heaterOn flag to true.
 */
void turnOnHeater()
{
    // Turn on the heater
    digitalWrite(HEATER_PIN, HIGH);
    heaterOn = true;
}
/**
 * @brief Turns off the heater
 *
 * This function will turn off the heater by setting the HEATER_PIN
 * low and setting the heaterOn flag to false.
 */
void turnOffHeater()
{
    // Turn off the heater
    digitalWrite(HEATER_PIN, LOW);
    heaterOn = false;
}

void turnOnCleaner()
{
    digitalWrite(CLEANER_PIN, HIGH);
    cleanerOn = true;
}

void turnOffCleaner()
{
    digitalWrite(CLEANER_PIN, LOW);
    cleanerOn = false;
}

// ===============================================================
// ===============================================================
// EEPROM functions
// ----------------
void saveSettings()
{
    EEPROM.put(0x0, g_setTemperatureF);
    EEPROM.put(0x8, g_timerSetting);
    EEPROM.put(0x10, g_contrast);
    EEPROM.commit();
}
/// @brief Load settings from EEPROM.  Apply defaults if not found
void loadSettings()
{

    debug("\tEntered loadSettings()...");

    EEPROM.get(0, g_setTemperatureF);
    EEPROM.get(8, g_timerSetting);
    EEPROM.get(10, g_contrast);

    if (g_setTemperatureF == 0)
    {
        g_setTemperatureF = 72;
    }
    if (g_timerSetting == 0)
    {
        g_timerSetting = 10;
    }
    if (g_contrast == 0)
    {
        g_contrast = 64;
    }
    // u8g2.setContrast(g_contrast); // this is done in setup after calling loadSettings()

    debug("\texiting loadSettings()");
}

void turnOffBacklight()
{
    digitalWrite(BACKLIGHT_PIN, LOW);
    debugln("Backlight off");
}

void turnOnBacklight()
{
    digitalWrite(BACKLIGHT_PIN, HIGH);
    debugln("Backlight on");
}
