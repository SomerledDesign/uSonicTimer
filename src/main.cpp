/**
 * @file main.cpp
 * @remarks uSonicTimer
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief an addition to an old, inexpensive Ultrasonic cleaner to include heating and timing
 * @version 0.6.1 (73)
 * @date 10/08/26
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
 *   0.6.0 (72) - 10/08/26 - big-digit run screens (timer holds while heating), Settings menu,
 *                           F/C units, backlight 0 = off, contrast shown as 20-100
 *   0.6.1 (73) - 10/08/26 - blocky big-digit font; encoder acceleration in Set contrast;
 *                           a press saves in every settings page
 *
 * DISCLAIMER:
 *   With this design, including both the hardware & software I offer no guarantee that it is bug
 *   free or reliable. So, if you decide to build one and you have problems or issues and/or causes
 *   damage/harm to you, others or property then you are on your own. This work is experimental.
 *
 */
// Firmware version, shown as "0.6.1 (73)": semantic version MAJOR.MINOR.PATCH plus a build number.
// Bump PATCH for fixes, MINOR for features; 1.0.0 once it is installed and in service.
// FW_BUILD goes up by 1 for every build flashed for testing and never resets.
#define FW_VERSION "0.6.1"
#define FW_BUILD 73
#define HW_REV "D" // PCB rev 1d
/**
 *  Physical pins listed for comparison to pcb.
 *  in version 1b pcb 2/2023 the CS is connected to ground
 *  GPIO15 is used for
 *  PCB rev 1d (9/2026): LCD CS on GPIO2 (10K pull-up), backlight on GPIO15 (10K pull-down),
 *  1-Wire on GPIO0 (4K7 pull-up), encoder CLK on GPIO16. Heater/cleaner SSRs are now switched
 *  low-side by NPN transistors (GPIO HIGH = ON, same logic as before).
 *
 *
 *  === ESP8266 Pinout ===
 *  --- Power/Reset ---
 *                    Physical
 *                      PIN
 *   •   Vcc             8
 *   •   GND            15
 *   •   RST             1
 *
 *  --- Available  I/O ---
 *   •   GPIO0     D3   18
 *   •   GPIO1     D10  22  (TXD0) \
 *   •   GPIO3     D9   21  (RXD0)  |  If using as inputs don't call Serial.begin()
 *   •   GPIO2     D4   17  (TXD1)
 *
 *   •   GPIO4     D2   19  (SDA)   \
 *                                   }--> I2C
 *   •   GPIO5     D1   20  (SCL)   /
 *
 *   ◎   GPIO6     ☞    14  USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO7     ☞    10  USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO8     ☞    13  USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO9     ☞    11  USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO10    ☞    12  USED INTERNALLY - DO NOT USE **
 *   ◎   GPIO11    ☞     9  USED INTERNALLY - DO NOT USE **
 *
 *   •   GPIO14    D5    5  (SCLK) | \ S
 *   ●   GPIO12    D6    6  (MISO) |  |  P
 *   •   GPIO13    D7    7  (MOSI) |  |    I
 *   •   GPIO15    D8   16  (CS)   | /      ..
 *   •   GPIO16    D0    4  (no interrupt)
 *   •   ADC0      A0    2  (Analog Input)
 *
 * ----------------------------------------------------------------------------------------
 *  Required I/O (PCB rev 1d)
 *    SPI Nokia 5110
 *      Func          Non i/o     Digitial     GPIO i/o     ESP12e Pin         Descr.                   Ω out
 *     =========================================================================================================
 *    1 RESET          RST        RST          -           1                  RESET line
 *    2 CS                        D4           GPIO2       17                 CHIP SELECT (10K pull-up)
 *    3 D/C                       D10/TX       GPIO1       22                 DATA|COMMAND
 *    4 DIN                       D7(MOSI)     GPIO13      7                  MOSI
 *    5 CLK                       D5(SCLK)     GPIO14      5                  SCLK
 *    6 VCC            3V
 *    7 BL                        D8           GPIO15      16                 Backlight (via Q1, 10K pull-down)
 *    8 GND
 *
 *    Rotary Encoder
 *      Func          Non i/o     Digitial     GPIO i/o     ESP12e Pin        Descr.         Ω out
 *     ===========================================================================================
 *      ROT_ENC_A_PIN             D0           GPIO16      4                  CLK (10K pull-up) ✓
 *      ROT_ENC_B_PIN             D6           GPIO12      6                  DT               ✓
 *      ROT_ENC_BUTTON_PIN        D9           GPIO3       21                 SW(/RX)           ✓
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
 *      ONE_WIRE_BUS              D3           GPIO0       18                  4K7 pull-up; also ~PROGRAM
 *
 *    ICSP Header (5 pin pogo pads)
 *      Func             Header pin    to -->                       ESP12e Pin    Ω out
 *     ===========================================================================================
 *     ICSP_GND_HDR_PIN      1        GND plane                       xx           ✓
 *     ICSP_RTS_HDR_PIN      2        Auto-Reset ckt.  (b Q3)         xx           ✓
 *     ICSP_TX_HDR_PIN       3        (this is tx in)                 22           ✓
 *     ICSP_RX_HDR_PIN       4        (this is _tx_ out)              21           ✓
 *     ICSP_DRT_HDR_PIN      5        Auto-Reset ckt.  (b Q2)         xx           ✓
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

#include <OneWire.h>
#include <DallasTemperature.h>
// #include <ESPRotary.h> // replaced by the polled decoder in pollEncoder()
#include <Button2.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include "Ticker.h" // https://github.com/esp8266/Arduino/tree/master/libraries/Ticker
#include "ust_logic.h" // run control (hysteresis), units, contrast and progress helpers
#include "screens.h"   // page drawing (big-digit run screens, menus, settings pages)

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

// Pin definitions (PCB rev 1d)
#define ONE_WIRE_BUS D3 // GPIO0 (4K7 pull-up, shared with ~PROGRAM)

#define CLEANER_PIN D1  // GPIO5
#define HEATER_PIN D2   // GPIO4

#define ROTARY_PIN1 D0     // GPIO16, CLK (polled; no interrupt on GPIO16)
#define ROTARY_PIN2 D6     // GPIO12, DT
#define ROTARY_BUTTON D9   // GPIO3, SW/RX

#define BACKLIGHT_PIN D8 // GPIO15 (10K pull-down, drives Q1)

#define LCD_SCLK_PIN D5   // GPIO14
#define LCD_DIN_PIN D7    // GPIO13
#define LCD_DC_PIN D10    // GPIO1 (TX)
#define LCD_CS_PIN D4      // GPIO2 (10K pull-up)
#define LCD_RST_PIN U8X8_PIN_NONE
// Status LED
// The backlight (PWM on GPIO15) is used as the status indicator, see updateBacklight():
// Steady       = normal operation (menu idle, cleaning) at the Set backlight level
// Flashing     = Heating up (1 Hz between the set brightness and ~30% of it)
// Pulsing      = timer finished, until any input (shows even if the backlight is toggled off)
// Double-blink = no temp sensor during a run, heater locked off (shows even if toggled off)
// Dim          = ~20% after 5 minutes idle in the menu; the first input only wakes it
// Heating/cleaning stay dark when the backlight has been toggled off or set to level 0.
// #define STATUS_LED_PIN ?   (not needed: the backlight is the status LED)
// bool statusLedOn = false;

// Encoder transitions per detent: 2 = half-cycle encoder (detents rest at both 00 and 11),
// 4 = full-cycle encoder (one rest state, e.g. always 11). A = B in every detent on the
// Rev D board; the encoder test screen (hold the button at power-up) shows which one it is.
#define ENCODER_STEPS_PER_DETENT 2

// Startup screen
#define FW_STR_(x) #x
#define FW_STR(x) FW_STR_(x)
#define STARTUP_LINE1 "uSonicTimer"
#define STARTUP_LINE2 "v" FW_VERSION " (" FW_STR(FW_BUILD) ")"
#define STARTUP_LINE3 "PCB Rev " HW_REV
#define STARTUP_SCREEN_MS 1500
// 84 px / 6 px font = 14 characters per line
static_assert(sizeof(STARTUP_LINE1) - 1 <= 14, "startup line 1 too long");
static_assert(sizeof(STARTUP_LINE2) - 1 <= 14, "startup line 2 too long");
static_assert(sizeof(STARTUP_LINE3) - 1 <= 14, "startup line 3 too long");
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
Ticker t;
Button2 b;

// Variables
float currentTemperature = 0;
uint8_t g_setTemperatureF;   // The set temperature in Fahrenheit
uint8_t g_timerSetting;      // The timer to be set in minutes
uint8_t g_contrast;          // The contrast for the display (raw U8g2 value 80..200)
bool g_unitsC = false;       // display unit: false = F, true = C (control logic is always F)
uint16_t longPress = 1000;   // one second long press of button
bool g_backlightOn = true;   // backlight state; toggled by a short press in the menus
uint8_t g_backlightLevel;    // backlight brightness level 0..10 (0 = off; Set backlight page)
uint8_t g_previewLevel = 0;  // level previewed while the Set backlight page is open
bool g_previewActive = false; // true while the Set backlight page previews g_previewLevel

// EEPROM layout (one byte per setting; addresses kept from earlier firmware so saved
// temperature/timer/contrast/backlight values survive the update)
#define EEPROM_SIZE           512
#define EEPROM_ADDR_SET_TEMP  0x00 // uint8_t g_setTemperatureF
#define EEPROM_ADDR_TIMER     0x08 // uint8_t g_timerSetting
#define EEPROM_ADDR_CONTRAST  0x10 // uint8_t g_contrast, raw 80..200 (shown as 20..100)
#define EEPROM_ADDR_BACKLIGHT 0x18 // uint8_t g_backlightLevel
#define EEPROM_ADDR_UNITS     0x20 // uint8_t UNITS_F / UNITS_C (new in 0.6.0)
#define EEPROM_ADDR_LAYOUT    0x28 // uint8_t EEPROM_LAYOUT_ID, written by 0.6.0 and later
#define EEPROM_LAYOUT_ID      0x60 // marks a layout that knows backlight level 0 and the units byte
#define UNITS_F               0
#define UNITS_C               1
static_assert(EEPROM_ADDR_SET_TEMP + sizeof(uint8_t) <= EEPROM_ADDR_TIMER, "EEPROM: set temp overlaps timer");
static_assert(EEPROM_ADDR_TIMER + sizeof(uint8_t) <= EEPROM_ADDR_CONTRAST, "EEPROM: timer overlaps contrast");
static_assert(EEPROM_ADDR_CONTRAST + sizeof(uint8_t) <= EEPROM_ADDR_BACKLIGHT, "EEPROM: contrast overlaps backlight");
static_assert(EEPROM_ADDR_BACKLIGHT + sizeof(uint8_t) <= EEPROM_ADDR_UNITS, "EEPROM: backlight overlaps units");
static_assert(EEPROM_ADDR_UNITS + sizeof(uint8_t) <= EEPROM_ADDR_LAYOUT, "EEPROM: units overlaps layout id");
static_assert(EEPROM_ADDR_LAYOUT + sizeof(uint8_t) <= EEPROM_SIZE, "EEPROM: layout id outside EEPROM");

// Valid setting ranges (erased flash reads 0xFF and a bare module may hold leftover bytes).
// Set temperature and contrast ranges are in ust_logic.h.
#define TIMER_MIN_MINUTES 1
#define TIMER_MAX_MINUTES 60
#define BACKLIGHT_MIN     0   // 0 = off (status pulse/double-blink still show, see updateBacklight())
#define BACKLIGHT_MAX     10
#define BACKLIGHT_DEFAULT 10  // = fully on, as before
#define BL_OFF_STATUS_LEVEL 5 // brightness of the done pulse / fault blink when the level is 0

// Backlight PWM and status effects
#define BACKLIGHT_PWM_FREQ   1000 // Hz; flicker-free, no whine, light interrupt load (timer1)
#define BL_LOW_PERCENT       30   // flash/pulse/blink low level, % of the set brightness
#define BL_DIM_PERCENT       20   // idle dim level, % of the set brightness
#define BL_UPDATE_MS         15   // status effect update interval
#define BL_FLASH_MS          1000 // heating flash period
#define BL_PULSE_MS          2500 // timer finished pulse period
#define BL_BLINK_MS          2000 // sensor fault double-blink period
#define IDLE_DIM_MS          (5UL * 60UL * 1000UL) // dim after 5 minutes idle in the menu
#define TEMP_CONVERSION_MS   800  // DS18B20 12-bit conversion takes 750ms

// PWM duty (0-255) for levels 1..10; roughly perceptual (the eye is far more sensitive at the low end)
static const uint8_t BACKLIGHT_LEVEL_DUTY[BACKLIGHT_MAX] = {4, 8, 14, 24, 38, 56, 80, 110, 160, 255};
static_assert(BL_OFF_STATUS_LEVEL >= 1 && BL_OFF_STATUS_LEVEL <= BACKLIGHT_MAX, "BL_OFF_STATUS_LEVEL out of range");

// What the backlight is currently showing
enum BacklightStatus
{
    BL_IDLE,     // steady (or dimmed after inactivity)
    BL_HEATING,  // flashing
    BL_CLEANING, // steady
    BL_DONE,     // pulsing until any input
    BL_FAULT     // double-blink: no temp sensor during a run
};
BacklightStatus g_blStatus = BL_IDLE;
unsigned long g_lastActivityMs = 0; // last encoder/button input
bool g_dimmed = false;              // idle dim active (main menu only)

// Button events (Button2 keeps wasPressed() set until read(), so every event must be consumed)
enum ButtonEvent
{
    BUTTON_NONE,
    BUTTON_SHORT,
    BUTTON_LONG
};
int16_t last = 0;            // For rotary encoder reading

// Rotary encoder decoder state (loop context only: never touched by the Ticker, so not volatile)
uint8_t g_encState = 0;      // last A/B sample, bit1 = A (CLK), bit0 = B (DT)
int8_t g_encCount = 0;       // transitions since the last detent
int8_t g_encLastDir = 0;     // direction of the last valid transition
int16_t g_encPosition = 0;   // detents turned since power-up
uint16_t g_encEdgesA = 0;    // raw A/B changes, shown on the encoder test screen
uint16_t g_encEdgesB = 0;
unsigned long g_encDetentMs = 0;          // when the last detent was counted
uint16_t g_encDetentGapMs = ENC_GAP_NONE; // ms since the detent before it (same direction)
int8_t g_encDetentDir = 0;                // direction of the last detent
uint8_t g_encSteps = 1;      // acceleration of the detent readRotaryEncoder() last returned
uint8_t g_buttonClicks = 0;  // clicks in the event readButton() last returned (2 = double click)

volatile bool down  = false; // Flags for the encoder
volatile bool up    = false;

// State variables
volatile bool cleanerOn = false; // State of the cleaner
volatile bool heaterOn = false;  // State of the heater

// Menu structure: main menu and the Settings... submenu (both drawn by displayMenu())
enum MenuLevel
{
    MENU_MAIN,
    MENU_SETTINGS
};
enum MainItems
{
    MAIN_START,
    MAIN_SETTINGS,
    MAIN_ITEMS_COUNT
};
enum SettingsItems
{
    SETTINGS_TEMP,
    SETTINGS_TIME,
    SETTINGS_UNITS,
    SETTINGS_BACKLIGHT,
    SETTINGS_CONTRAST,
    SETTINGS_EXIT,
    SETTINGS_ITEMS_COUNT
};
uint8_t g_menuLevel = MENU_MAIN;
uint8_t g_currentMenu = MAIN_START;      // highlighted main menu item
uint8_t g_settingsItem = SETTINGS_TEMP;  // highlighted Settings item

// Function definitions
void startTimerPage();
void showDonePage();
void setTimerSubmenu();
void setTemperatureSubmenu();
void setUnitsSubmenu();
void adjustContrast();
void setBacklightSubmenu();
bool backlightLit();
const char *unitSuffix();
void displayMenu();
void saveSettings();
void loadSettings();
void handleLoop();
void buttonTick();
void pollEncoder();
void encoderTestPage();
void showStartupScreen();
void servicePage();
void sendBufferPolled();
void readRotaryEncoder();
void flushEncoder();
void turnOnHeater();
void turnOffHeater();
void turnOnCleaner();
void turnOffCleaner();
void turnOnBacklight();
void turnOffBacklight();
void toggleBacklight();
void updateBacklight(bool force = false);
void setBacklightDuty(uint8_t duty);
void noteActivity();
ButtonEvent readButton();

void setup()
{
    debugbegin(115200);
    debug("Entered setup()...");

    // Initialize EEPROM for saving settings
    ///////////////////////////////////////////////////////////////
    EEPROM.begin(EEPROM_SIZE);

    // delay(4000);

    loadSettings(); // Load settings from EEPROM

    // Initialize display
    ///////////////////////////////////////////////////////////////
    u8g2.begin();
    u8g2.setFontMode(1); // transparent glyphs: inverse (colour 0) text only clears the strokes
    u8g2.setContrast(g_contrast);

    // Initialize temperature sensor
    ///////////////////////////////////////////////////////////////
    sensors.begin();

    // Initialize rotary encoder (polled by pollEncoder() from loop context)
    ///////////////////////////////////////////////////////////////
    // GPIO16 has no internal pull-up (R9 4K7 is external) and the core's pinMode(16, INPUT_PULLUP)
    // leaves the GPIO16 output enable untouched; INPUT explicitly makes it an input
    pinMode(ROTARY_PIN1, INPUT);
    pinMode(ROTARY_PIN2, INPUT_PULLUP); // R10 4K7 external as well
    g_encState = (digitalRead(ROTARY_PIN1) << 1) | digitalRead(ROTARY_PIN2);
    last = g_encPosition;
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
    t.attach_ms(10, buttonTick); // poll the button every 10ms (the encoder is only read in loop context)

    // Initialize pins
    ///////////////////////////////////////////////////////////////

    pinMode(BACKLIGHT_PIN, OUTPUT); // GPIO15 is held LOW by its pull-down until here (boot strap)
    analogWriteRange(255);
    analogWriteFreq(BACKLIGHT_PWM_FREQ);
    turnOnBacklight(); // backlight starts on at the saved level; a short press on the main menu toggles it
    g_lastActivityMs = millis();
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(CLEANER_PIN, OUTPUT);
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(CLEANER_PIN, LOW);

    // startup screen (name, version, board revision) for STARTUP_SCREEN_MS
    const bool testHeld = (digitalRead(ROTARY_BUTTON) == LOW); // checked before the startup screen
    const unsigned long startupMs = millis();
    showStartupScreen();

    // Read initial temperature (while the startup screen is shown)
    sensors.requestTemperatures();
    currentTemperature = sensors.getTempFByIndex(0); // TODO: get address of sensor and use that instead

    while (millis() - startupMs < STARTUP_SCREEN_MS)
    {
        servicePage(); // keeps the watchdog fed
    }

    // hold the encoder button during power-up/reset for the encoder test screen
    if (testHeld || digitalRead(ROTARY_BUTTON) == LOW)
    {
        encoderTestPage();
    }
    last = g_encPosition; // ignore any turning during the startup screen

    // TODO: setup wifi 
    // this is a no-op for now, but could be implented in the future to allow for OTA updates, remote monitoring, etc.

    // TODO: setup ntp
    // incorporate easy NTP TZ DST.cpp
    // also a no-op for now, but could be implemented in the future to allow for time-based operations, logging, etc.

    debugln("...setup Complete.");
}

void loop()
{

    debugln("Entering loop()...");

    handleLoop(); // poll the encoder/button every pass, not only from the 10ms Ticker
    updateBacklight();

    displayMenu();

    // debug("displayMenu complete...");

    readRotaryEncoder();
    const ButtonEvent event = readButton();

    // debug("rotaryEncoder has been read...");

    // idle dim: the first input after dimming only wakes the backlight
    if (g_dimmed)
    {
        if (down || up || event != BUTTON_NONE)
        {
            down = false;
            up = false;
            last = g_encPosition; // drop any further detents of the waking turn
            g_dimmed = false;
            updateBacklight(true);
        }
        return;
    }
    if (backlightLit() && g_blStatus == BL_IDLE && (millis() - g_lastActivityMs) >= IDLE_DIM_MS)
    {
        g_dimmed = true;
    }

    // handle encoder events: move the highlight in the current menu (wraps around)
    ////////////////////////////////////////////
    const bool inSettings = (g_menuLevel == MENU_SETTINGS);
    const uint8_t count = inSettings ? static_cast<uint8_t>(SETTINGS_ITEMS_COUNT) : static_cast<uint8_t>(MAIN_ITEMS_COUNT);
    uint8_t &selected = inSettings ? g_settingsItem : g_currentMenu;
    if (down)
    {
        debug("down rotate...");
        down = false;
        selected = (selected + 1) % count;
    }
    if (up)
    {
        debug("up rotate...");
        up = false;
        selected = (selected + count - 1) % count;
    }

    // handle button events: long press opens the highlighted item
    ////////////////////////////////////////////
    if (event == BUTTON_LONG)
    {
        if (!inSettings)
        {
            switch (g_currentMenu)
            {
            case MAIN_START:
                debug("long press on Start...");
                startTimerPage();
                break;
            case MAIN_SETTINGS:
                g_menuLevel = MENU_SETTINGS;
                g_settingsItem = SETTINGS_TEMP;
                break;
            default:
                break;
            }
        }
        else
        {
            switch (g_settingsItem)
            {
            case SETTINGS_TEMP:
                setTemperatureSubmenu();
                break;
            case SETTINGS_TIME:
                setTimerSubmenu();
                break;
            case SETTINGS_UNITS:
                setUnitsSubmenu();
                break;
            case SETTINGS_BACKLIGHT:
                setBacklightSubmenu();
                break;
            case SETTINGS_CONTRAST:
                adjustContrast();
                break;
            default: // SETTINGS_EXIT
                g_menuLevel = MENU_MAIN;
                break;
            }
        }
        debugln("Loop complete.");
    }
    else if (event == BUTTON_SHORT)
    {
        // short press in the menus toggles the backlight;
        // pages use short presses for their own purposes and leave it alone
        toggleBacklight();
    }
}

/**
 * @brief Runs the cleaner: heats first, then counts down while cleaning.
 *
 * Called when the user selects "Start". The countdown holds while the heater is on and only
 * runs while the water is within 10 F of the set temperature (cleaner on), see nextRunState():
 *   heating  -> heater on, cleaner off, countdown held; big current temperature on screen
 *   cleaning -> heater off, cleaner on, counting down; big MM:SS on screen
 *   The heater comes on below (set - 10 F) and cleaning resumes at (set - 8 F).
 *   no sensor -> fail-safe: heater off, cleaner on, counting down; "NO SENSOR" under the time
 * A long press aborts. When the time is up both outputs go off and the DONE screen stays
 * until any input.
 */
void startTimerPage()
{
    const uint32_t totalMs = static_cast<uint32_t>(g_timerSetting) * 60UL * 1000UL;
    uint32_t elapsedMs = 0;   // run time; only advances while cleaning (or without a sensor)
    bool finished = false;
    RunState state = RUN_START;
    float heatStartF = 0.0f;  // temperature when the heater (re)engaged, for the progress rule
    char line[24];

    // one blocking read so the first heater decision uses a fresh temperature, then non-blocking
    // conversions so the page (encoder, button, backlight status) keeps running during the 750ms
    sensors.requestTemperatures();
    currentTemperature = sensors.getTempFByIndex(0);
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    unsigned long lastRequest = millis();
    unsigned long lastTick = millis();

    while (true)
    {
        servicePage();
        const unsigned long now = millis();
        // the time since the last pass counts only if that pass was cleaning (or had no sensor)
        if (runStateCounts(state))
        {
            elapsedMs += now - lastTick;
        }
        lastTick = now;

        if (elapsedMs >= totalMs)
        {
            finished = true;
            break;
        }

        if (now - lastRequest >= TEMP_CONVERSION_MS)
        {
            currentTemperature = sensors.getTempFByIndex(0);
            sensors.requestTemperatures();
            lastRequest = millis();
        }
        // no sensor (DEVICE_DISCONNECTED_F = -196.6F) must never leave the heater on
        const bool sensorOk = currentTemperature > (DEVICE_DISCONNECTED_F + 1.0f);

        const RunState next = nextRunState(state, sensorOk, currentTemperature, g_setTemperatureF);
        if (next == RUN_HEATING && state != RUN_HEATING)
        {
            heatStartF = currentTemperature; // progress rule starts from here
        }
        state = next;

        // switch the output that goes off first, so heater and cleaner are never on together
        switch (state)
        {
        case RUN_HEATING:
            turnOffCleaner();
            turnOnHeater();
            g_blStatus = BL_HEATING;
            break;
        case RUN_NO_SENSOR:
            // fail-safe: heater off, cleaner still runs for the timer (cleaning without temp control)
            turnOffHeater();
            turnOnCleaner();
            g_blStatus = BL_FAULT;
            break;
        default: // RUN_CLEANING
            turnOffHeater();
            turnOnCleaner();
            g_blStatus = BL_CLEANING;
            break;
        }

        const int16_t setShown = displayTemp(g_setTemperatureF, g_unitsC);
        if (state == RUN_HEATING)
        {
            snprintf(line, sizeof(line), "Heating to %d%s", setShown, unitSuffix());
            drawHeatingScreen(u8g2, displayTemp(currentTemperature, g_unitsC), g_unitsC,
                              heatProgressPx(heatStartF, currentTemperature, g_setTemperatureF, LCD_WIDTH), line);
        }
        else
        {
            formatTimerLine(u8g2, line, sizeof(line), sensorOk, displayTemp(currentTemperature, g_unitsC), setShown, g_unitsC);
            // colon blinks at 1 Hz with the counted time
            drawTimerScreen(u8g2, remainingSeconds(totalMs, elapsedMs), (elapsedMs % 1000UL) < 500UL,
                            progressPx(static_cast<int32_t>(elapsedMs), static_cast<int32_t>(totalMs), LCD_WIDTH), line);
        }
        sendBufferPolled();

        // long press aborts the timer
        if (readButton() == BUTTON_LONG)
        {
            break;
        }
    }
    sensors.setWaitForConversion(true);
    turnOffCleaner();
    turnOffHeater();
    g_blStatus = finished ? BL_DONE : BL_IDLE; // pulse until any input when the timer ran out
    g_lastActivityMs = millis();
    last = g_encPosition; // ignore any turning done while the timer page was shown
    down = false;
    up = false;
    if (finished)
    {
        showDonePage();
    }
}

/**
 * @brief Big "DONE" screen after a completed run; any turn or press returns to the menu.
 *
 * The backlight keeps pulsing (BL_DONE) until that input, see noteActivity().
 */
void showDonePage()
{
    char line[20];
    snprintf(line, sizeof(line), "Cleaned %u min", static_cast<unsigned>(g_timerSetting));
    drawDoneScreen(u8g2, line);
    sendBufferPolled();
    while (true)
    {
        servicePage();
        readRotaryEncoder();
        const ButtonEvent event = readButton();
        if (down || up || event != BUTTON_NONE)
        {
            break;
        }
    }
    down = false;
    up = false;
    last = g_encPosition;
}

/**
 * @brief Shows the timer selection page (Settings > Set time)
 *
 * The user can cycle through the available preset times (3, 8, 10, 15, 20, 30, 60 minutes)
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
        servicePage();
        readRotaryEncoder();
        char value[12];
        snprintf(value, sizeof(value), "%u min", static_cast<unsigned>(presets[selectedIndex]));
        drawValuePage(u8g2, "Set time", value, 0, 0, "Press = save");
        sendBufferPolled();

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

        if (readButton() != BUTTON_NONE)
        {
            g_timerSetting = presets[selectedIndex];
            saveSettings();
            break;
        }
    }
    flushEncoder();
}

/**
 * @brief Shows the temperature selection page (Settings > Set temp)
 *
 * Edits the set temperature in the display unit: 3 digits in F (60..180), 2 in C (16..82).
 * Rotating changes the cursor digit (reverse video, 0-9, wraps). A short press moves to the
 * next digit; a short press on the last digit saves and exits. A long press saves at any time.
 * The value is always stored in whole F; a C value that was not changed leaves the stored
 * F value as it was.
 */
void setTemperatureSubmenu()
{
    const bool celsius = g_unitsC;
    const uint8_t n = celsius ? 2 : 3;
    const int16_t shown = displayTemp(g_setTemperatureF, celsius);
    uint8_t cursorPosition = 0;

    // split the value into digits, most significant first (e.g. 123 -> {1, 2, 3})
    uint8_t digits[3] = {0, 0, 0};
    int16_t v = (shown < 0) ? 0 : shown;
    for (int8_t i = n - 1; i >= 0; i--)
    {
        digits[i] = static_cast<uint8_t>(v % 10);
        v /= 10;
    }

    while (true)
    {
        servicePage();
        readRotaryEncoder();
        drawDigitEditor(u8g2, "Set temp", digits, n, cursorPosition, unitSuffix(),
                        (cursorPosition + 1 < n) ? "Press = next" : "Press = save");
        sendBufferPolled();

        if (down)
        {
            down = false;
            digits[cursorPosition] = (digits[cursorPosition] + 1) % 10;
        }
        else if (up)
        {
            up = false;
            digits[cursorPosition] = (digits[cursorPosition] + 10 - 1) % 10;
        }

        const ButtonEvent event = readButton();
        bool save = (event == BUTTON_LONG); // long press saves from any digit

        // short press moves to the next digit; on the last digit it saves
        // (a double click, which Button2 reports as one event, moves two digits)
        if (event == BUTTON_SHORT)
        {
            for (uint8_t k = 0; k < g_buttonClicks && !save; k++)
            {
                if (cursorPosition + 1 >= n)
                {
                    save = true;
                }
                else
                {
                    cursorPosition++;
                }
            }
        }

        if (save)
        {
            int16_t newValue = 0;
            for (uint8_t i = 0; i < n; i++)
            {
                newValue = newValue * 10 + digits[i];
            }
            if (newValue != shown)
            {
                g_setTemperatureF = celsius ? setTempFromC(newValue)
                                            : static_cast<uint8_t>(constrain(newValue, SET_TEMP_MIN_F, SET_TEMP_MAX_F));
            }
            saveSettings();
            break;
        }
    }
    flushEncoder();
}

/**
 * @brief Chooses the display unit (Settings > Set units): rotate toggles F/C, any press saves.
 *
 * Only the display changes; control stays in F and the set temperature is stored in F.
 */
void setUnitsSubmenu()
{
    bool celsius = g_unitsC;
    while (true)
    {
        servicePage();
        readRotaryEncoder();
        drawValuePage(u8g2, "Set units", celsius ? DEG_SIGN "C Celsius" : DEG_SIGN "F Fahrenheit", 0, 0, "Press = save");
        sendBufferPolled();

        if (down || up)
        {
            down = false;
            up = false;
            celsius = !celsius;
        }

        if (readButton() != BUTTON_NONE)
        {
            if (celsius != g_unitsC)
            {
                g_unitsC = celsius;
                saveSettings();
            }
            break;
        }
    }
    flushEncoder();
}

/**
 * @brief Adjusts the display contrast (Settings > Set contrast)
 *
 * Shows the contrast as 20..100, mapped linearly onto the usable raw range 80..200
 * (outside it the display blanks). Clockwise increases, with acceleration (a quick turn moves
 * 5 per detent, a medium one 2, single clicks 1); applied live. Any press (short or long) saves
 * and exits. Leaving the value unchanged keeps the exact raw value saved before.
 */
void adjustContrast()
{
    const uint8_t startUi = contrastRawToUi(g_contrast);
    uint8_t ui = startUi;
    uint8_t applied = g_contrast;
    while (true)
    {
        servicePage();
        readRotaryEncoder();
        char value[6];
        snprintf(value, sizeof(value), "%u", static_cast<unsigned>(ui));
        drawValuePage(u8g2, "Set contrast", value, ui - CONTRAST_UI_MIN, CONTRAST_UI_MAX - CONTRAST_UI_MIN, "Press = save");
        sendBufferPolled();

        // clockwise (down) = increase, same as the other pages; accelerated by g_encSteps
        if (down)
        {
            down = false;
            ui = (ui + g_encSteps < CONTRAST_UI_MAX) ? static_cast<uint8_t>(ui + g_encSteps) : CONTRAST_UI_MAX;
        }
        else if (up)
        {
            up = false;
            ui = (ui > CONTRAST_UI_MIN + g_encSteps) ? static_cast<uint8_t>(ui - g_encSteps) : CONTRAST_UI_MIN;
        }
        const uint8_t raw = (ui == startUi) ? g_contrast : contrastUiToRaw(ui);
        if (raw != applied)
        {
            u8g2.setContrast(raw);
            applied = raw;
        }

        // any press saves and exits
        if (readButton() != BUTTON_NONE)
        {
            if (raw != g_contrast)
            {
                g_contrast = raw;
                saveSettings();
            }
            break;
        }
    }
    flushEncoder();
}

/**
 * @brief Adjusts the backlight brightness (Settings > Set backlight)
 *
 * Rotating the encoder changes the level (0..10, 0 = off, clockwise = brighter) and previews
 * it live, one level per detent (no acceleration for 11 levels). Any press (short or long) saves
 * the level to EEPROM, switches the backlight on and exits.
 */
void setBacklightSubmenu()
{
    uint8_t level = g_backlightLevel;
    while (true)
    {
        g_previewLevel = level; // live preview, see updateBacklight()
        g_previewActive = true;
        servicePage();
        readRotaryEncoder();
        char value[10];
        if (level == 0)
        {
            snprintf(value, sizeof(value), "Off");
        }
        else
        {
            snprintf(value, sizeof(value), "Level %u", static_cast<unsigned>(level));
        }
        drawValuePage(u8g2, "Set backlight", value, level, BACKLIGHT_MAX, "Press = save");
        sendBufferPolled();

        if (down)
        {
            down = false;
            if (level < BACKLIGHT_MAX)
            {
                level++;
            }
        }
        else if (up)
        {
            up = false;
            if (level > BACKLIGHT_MIN)
            {
                level--;
            }
        }

        // any press saves and exits
        if (readButton() != BUTTON_NONE)
        {
            g_backlightLevel = level;
            saveSettings();
            g_backlightOn = true;
            break;
        }
    }
    g_previewActive = false;
    updateBacklight(true);
    flushEncoder();
}

/**
 * @brief Draws the current menu (main menu or Settings...)
 *
 * Title bar in reverse video, the items in 6x10 with the highlighted one in reverse video.
 * The Settings list shows 4 rows and scrolls (arrows mark more items above/below). The main
 * menu shows the set temperature and time underneath.
 */
void displayMenu()
{
    static const char *const mainItems[MAIN_ITEMS_COUNT] = {"Start", "Settings..."};
    static const char *const settingsItems[SETTINGS_ITEMS_COUNT] = {
        "Set temp", "Set time", "Set units", "Set backlight", "Set contrast", "Exit"};
    static uint8_t mainFirstRow = 0;
    static uint8_t settingsFirstRow = 0;

    if (g_menuLevel == MENU_SETTINGS)
    {
        drawMenuList(u8g2, "Settings", settingsItems, SETTINGS_ITEMS_COUNT, g_settingsItem, settingsFirstRow, nullptr);
    }
    else
    {
        char footer[20];
        snprintf(footer, sizeof(footer), "%d%s  %u min", displayTemp(g_setTemperatureF, g_unitsC), unitSuffix(),
                 static_cast<unsigned>(g_timerSetting));
        drawMenuList(u8g2, "uSonicTimer", mainItems, MAIN_ITEMS_COUNT, g_currentMenu, mainFirstRow, footer);
    }
    sendBufferPolled();
}

/// " F" / " C" with the degree sign, for the current display unit
const char *unitSuffix()
{
    return g_unitsC ? DEG_SIGN "C" : DEG_SIGN "F";
}

/**
 * @brief Handles the loop tasks for the rotary encoder and button.
 *
 * Call this method repeatedly (loop context only: loop(), servicePage(), sendBufferPolled())
 * to handle the rotary encoder and button events.
 */
void handleLoop()
{
    pollEncoder();
    b.loop();
}

/**
 * @brief 10ms Ticker callback: button only. The encoder is never read from the Ticker.
 */
void buttonTick()
{
    b.loop();
}

/**
 * @brief Quadrature decoder for the rotary encoder (replaces ESPRotary).
 *
 * Reads A (GPIO16) and B (GPIO12) directly and counts Gray-code transitions. Detents are
 * only counted in a rest state (A == B), so contact bounce cancels out and the count lines
 * up with the detents by itself. A sample that skipped the middle state (both pins changed)
 * counts as two steps in the direction of the last valid one. Never yields; call it often.
 */
void pollEncoder()
{
    // index = old state << 2 | new state; +1/-1 per valid transition, 2 = both pins changed
    static const int8_t ENC_TABLE[16] = {0, 1, -1, 2, -1, 0, 2, 1, 1, 2, 0, -1, 2, -1, 1, 0};
    const uint8_t s = (digitalRead(ROTARY_PIN1) << 1) | digitalRead(ROTARY_PIN2);
    if (s == g_encState)
    {
        return;
    }
    if ((s ^ g_encState) & 0x02)
    {
        g_encEdgesA++;
    }
    if ((s ^ g_encState) & 0x01)
    {
        g_encEdgesB++;
    }
    int8_t step = ENC_TABLE[(g_encState << 2) | s];
    if (step == 2)
    {
        step = 2 * g_encLastDir; // missed a state: assume it kept turning the same way
    }
    else
    {
        g_encLastDir = step;
    }
    g_encState = s;
    g_encCount += step;

    if (s == 0x00 || s == 0x03) // rest state (A == B)
    {
        const int16_t before = g_encPosition;
        while (g_encCount >= ENCODER_STEPS_PER_DETENT)
        {
            g_encCount -= ENCODER_STEPS_PER_DETENT;
            g_encPosition++;
        }
        while (g_encCount <= -ENCODER_STEPS_PER_DETENT)
        {
            g_encCount += ENCODER_STEPS_PER_DETENT;
            g_encPosition--;
        }
        if (g_encPosition != before)
        {
            // time between detents, for the acceleration in the value editors
            const int8_t dir = (g_encPosition > before) ? 1 : -1;
            const unsigned long now = millis();
            const unsigned long gap = now - g_encDetentMs;
            g_encDetentGapMs = (dir == g_encDetentDir && gap < ENC_GAP_NONE) ? static_cast<uint16_t>(gap) : ENC_GAP_NONE;
            g_encDetentMs = now;
            g_encDetentDir = dir;
        }
    }
}

/**
 * @brief Services the encoder/button and lets the ESP8266 core run.
 *
 * Call this in every blocking page loop (while (true)). The yield() feeds the soft
 * watchdog (otherwise it resets the board after ~3 s) and lets the 10ms Ticker fire.
 * Never call it from the Ticker callback: yield() is not allowed in SYS context.
 */
void servicePage()
{
    handleLoop();
    updateBacklight();
    yield();
}

/**
 * @brief Sends the frame buffer to the LCD in small pieces, polling the encoder/button in between.
 *
 * A full software-SPI sendBuffer() blocks for roughly 14ms. The Ticker cannot run during it,
 * so the encoder was only sampled about once per redraw and a detent that snapped through all
 * of its transitions between two samples was never seen. Sending 18 pieces of 4x1 tiles keeps
 * the gap between encoder samples under ~1ms. Safe in any context (no yield()).
 */
void sendBufferPolled()
{
    const uint8_t tilesWide = u8g2.getBufferTileWidth();  // 11 tiles (88 px, 84 visible)
    const uint8_t tilesHigh = u8g2.getBufferTileHeight(); // 6 tiles (48 px)
    for (uint8_t ty = 0; ty < tilesHigh; ty++)
    {
        for (uint8_t tx = 0; tx < tilesWide; tx += 4)
        {
            const uint8_t tw = (tilesWide - tx < 4) ? (tilesWide - tx) : 4;
            u8g2.updateDisplayArea(tx, ty, tw, 1);
            handleLoop();
        }
    }
}

/**
 * @brief Turns the next pending detent into the down (clockwise) or up flag.
 *
 * Also sets g_encSteps, the acceleration for that detent (1, 2 or 5, see encoderAccelSteps()).
 * Only the value editors use g_encSteps; menus and the other pages move one step per detent.
 */
void readRotaryEncoder()
{
    const int16_t position = g_encPosition;
    g_encSteps = encoderAccelSteps(g_encDetentGapMs);

    // consume one detent per call so steps that arrive during a redraw are not dropped
    if (position > last)
    {
        last++;
        down = true;
        noteActivity();
    }
    else if (position < last)
    {
        last--;
        up = true;
        noteActivity();
    }
}

/**
 * @brief Drops detents still queued when a page closes, so they don't move the menu highlight.
 */
void flushEncoder()
{
    last = g_encPosition;
    down = false;
    up = false;
}

/**
 * @brief Startup screen: name, firmware version (build) and PCB revision.
 *
 * Only draws; setup() keeps it up for STARTUP_SCREEN_MS while servicing the encoder/button.
 */
void showStartupScreen()
{
    // centre each line on the 84 px display
    static const char *const lines[] = {STARTUP_LINE1, STARTUP_LINE2, STARTUP_LINE3};
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setDrawColor(1);
    for (uint8_t i = 0; i < 3; i++)
    {
        const int16_t x = (84 - u8g2.getStrWidth(lines[i])) / 2;
        u8g2.drawStr(x < 0 ? 0 : x, 12 + (i * 14), lines[i]);
    }
    sendBufferPolled();
}

/**
 * @brief Encoder test screen: hold the encoder button during power-up/reset to get here.
 *
 * Shows the live A (GPIO16) and B (GPIO12) levels, the detent count and how often each input
 * has changed. Turning should make both edge counts rise together. An edge count that stays
 * at 0 means that input never reaches the ESP8266 (check the module pad/trace), not firmware.
 * Resting levels that alternate 00/11 between detents mean ENCODER_STEPS_PER_DETENT 2.
 * Long press exits to the main menu.
 */
void encoderTestPage()
{
    // wait for the power-up press to be released, then drop that press
    while (digitalRead(ROTARY_BUTTON) == LOW)
    {
        servicePage();
    }
    const unsigned long released = millis();
    while (millis() - released < 100)
    {
        servicePage();
    }
    readButton();

    while (true)
    {
        servicePage();
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setDrawColor(1);
        u8g2.setCursor(0, 10);
        u8g2.print("A:");
        u8g2.print(g_encState >> 1);
        u8g2.print(" B:");
        u8g2.print(g_encState & 0x01);
        u8g2.print(" D:");
        u8g2.print(g_encPosition);
        u8g2.setCursor(0, 20);
        u8g2.print("A edges: ");
        u8g2.print(g_encEdgesA);
        u8g2.setCursor(0, 30);
        u8g2.print("B edges: ");
        u8g2.print(g_encEdgesB);
        u8g2.setCursor(0, 40);
        u8g2.print("Long: exit");
        sendBufferPolled();

        if (readButton() == BUTTON_LONG)
        {
            break;
        }
    }
    last = g_encPosition;
    down = false;
    up = false;
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
    EEPROM.put(EEPROM_ADDR_SET_TEMP, g_setTemperatureF);
    EEPROM.put(EEPROM_ADDR_TIMER, g_timerSetting);
    EEPROM.put(EEPROM_ADDR_CONTRAST, g_contrast);
    EEPROM.put(EEPROM_ADDR_BACKLIGHT, g_backlightLevel);
    EEPROM.put(EEPROM_ADDR_UNITS, static_cast<uint8_t>(g_unitsC ? UNITS_C : UNITS_F));
    EEPROM.put(EEPROM_ADDR_LAYOUT, static_cast<uint8_t>(EEPROM_LAYOUT_ID));
    EEPROM.commit();
}
/// @brief Load settings from EEPROM.  Apply defaults if not found
void loadSettings()
{

    debug("\tEntered loadSettings()...");

    EEPROM.get(EEPROM_ADDR_SET_TEMP, g_setTemperatureF);
    EEPROM.get(EEPROM_ADDR_TIMER, g_timerSetting);
    EEPROM.get(EEPROM_ADDR_CONTRAST, g_contrast);
    EEPROM.get(EEPROM_ADDR_BACKLIGHT, g_backlightLevel);
    uint8_t units = UNITS_F;
    uint8_t layout = 0;
    EEPROM.get(EEPROM_ADDR_UNITS, units);
    EEPROM.get(EEPROM_ADDR_LAYOUT, layout);
    // settings saved by 0.5.0 and earlier have no layout id; their values carry over as they are
    const bool layoutKnown = (layout == EEPROM_LAYOUT_ID);

    // apply defaults to anything out of range, not just 0 (erased flash reads 0xFF)
    if (g_setTemperatureF < SET_TEMP_MIN_F || g_setTemperatureF > SET_TEMP_MAX_F)
    {
        g_setTemperatureF = 72;
    }
    if (g_timerSetting < TIMER_MIN_MINUTES || g_timerSetting > TIMER_MAX_MINUTES)
    {
        g_timerSetting = 10;
    }
    // contrast is still stored raw (80..200), so a 0.5.0 value carries over; the page shows 20..100
    if (g_contrast < CONTRAST_MIN || g_contrast > CONTRAST_MAX)
    {
        g_contrast = CONTRAST_DEFAULT;
    }
    // level 0 (off) is only valid once 0.6.0 has saved; before that a 0 is a blank/stray byte
    if (g_backlightLevel > BACKLIGHT_MAX || (g_backlightLevel == 0 && !layoutKnown))
    {
        g_backlightLevel = BACKLIGHT_DEFAULT;
    }
    // anything but UNITS_C (including a blank 0xFF byte) means F, the default
    g_unitsC = layoutKnown && (units == UNITS_C);
    // u8g2.setContrast(g_contrast); // this is done in setup after calling loadSettings()

    debug("\texiting loadSettings()");
}

/**
 * @brief Returns (and consumes) the last completed button event.
 *
 * Button2 leaves wasPressed() set until read() is called, so the event is cleared here to
 * avoid acting on the same press again on the next pass or in the next menu page.
 * A press held longer than longPress counts as BUTTON_LONG, anything shorter as BUTTON_SHORT.
 * Button2 reports quick repeated presses as one event; g_buttonClicks holds how many.
 */
ButtonEvent readButton()
{
    if (!b.wasPressed())
    {
        return BUTTON_NONE;
    }
    g_buttonClicks = b.getNumberOfClicks();
    if (g_buttonClicks == 0)
    {
        g_buttonClicks = 1;
    }
    b.read(); // consume the event
    noteActivity();
    return (b.wasPressedFor() > longPress) ? BUTTON_LONG : BUTTON_SHORT;
}

/// true when the backlight shows at all in normal operation (on, and level 1..10)
bool backlightLit()
{
    return g_backlightOn && g_backlightLevel > 0;
}

void toggleBacklight()
{
    if (g_backlightOn)
    {
        turnOffBacklight();
    }
    else
    {
        turnOnBacklight();
    }
}

void turnOffBacklight()
{
    g_backlightOn = false;
    updateBacklight(true);
    debugln("Backlight off");
}

void turnOnBacklight()
{
    g_backlightOn = true;
    updateBacklight(true);
    debugln("Backlight on");
}

/**
 * @brief Records encoder/button input (idle dim timer) and acknowledges "timer finished".
 */
void noteActivity()
{
    g_lastActivityMs = millis();
    if (g_blStatus == BL_DONE)
    {
        g_blStatus = BL_IDLE;
    }
}

/**
 * @brief Drives the backlight PWM from the on/off state, brightness level and status.
 *
 * Non-blocking; call it often (loop() and servicePage()). Effects swing between the set
 * brightness and BL_LOW_PERCENT of it, so the screen stays readable. Timer finished and
 * sensor fault also show when the backlight is toggled off or set to level 0 (then at
 * BL_OFF_STATUS_LEVEL); heating/cleaning do not.
 * Only call it from loop context: analogWrite() can yield, which is not allowed in the Ticker.
 */
void updateBacklight(bool force)
{
    static unsigned long lastUpdate = 0;
    const unsigned long now = millis();
    if (!force && (now - lastUpdate) < BL_UPDATE_MS)
    {
        return;
    }
    lastUpdate = now;

    const uint8_t level = g_previewActive ? g_previewLevel : g_backlightLevel;
    const bool lit = g_backlightOn && level > 0; // level 0 = off, like toggled off
    const uint8_t full = BACKLIGHT_LEVEL_DUTY[((level > 0) ? level : BL_OFF_STATUS_LEVEL) - 1];
    uint8_t low = (full * BL_LOW_PERCENT) / 100;
    if (low < 1)
    {
        low = 1; // never fully off while an effect is running
    }
    uint8_t duty;

    if (g_previewActive)
    {
        duty = (level > 0) ? full : 0; // Set backlight page: show the level being chosen
    }
    else
    {
        switch (g_blStatus)
        {
        case BL_FAULT:
        {
            // two short blinks every BL_BLINK_MS
            const uint16_t phase = now % BL_BLINK_MS;
            const bool blink = (phase < 150) || (phase >= 300 && phase < 450);
            if (lit)
            {
                duty = blink ? low : full;
            }
            else
            {
                duty = blink ? full : 0;
            }
            break;
        }
        case BL_DONE:
        {
            // slow triangle pulse between the low level (or off) and the set brightness
            const uint8_t base = lit ? low : 0;
            const uint16_t half = BL_PULSE_MS / 2;
            const uint16_t phase = now % BL_PULSE_MS;
            const uint16_t ramp = (phase < half) ? phase : (BL_PULSE_MS - phase);
            duty = base + static_cast<uint8_t>((static_cast<uint32_t>(full - base) * ramp) / half);
            break;
        }
        case BL_HEATING:
            if (!lit)
            {
                duty = 0;
            }
            else
            {
                duty = ((now % BL_FLASH_MS) < (BL_FLASH_MS / 2)) ? full : low;
            }
            break;
        default: // BL_IDLE, BL_CLEANING
            if (!lit)
            {
                duty = 0;
            }
            else if (g_dimmed)
            {
                duty = (full * BL_DIM_PERCENT) / 100;
                if (duty < 1)
                {
                    duty = 1;
                }
            }
            else
            {
                duty = full;
            }
            break;
        }
    }
    setBacklightDuty(duty);
}

/**
 * @brief Writes the backlight PWM duty (0-255), only when it changes.
 *
 * All backlight writes go through here: a digitalWrite() on the pin would stop the PWM.
 */
void setBacklightDuty(uint8_t duty)
{
    static int16_t lastDuty = -1;
    if (duty == lastDuty)
    {
        return;
    }
    lastDuty = duty;
    analogWrite(BACKLIGHT_PIN, duty);
}
