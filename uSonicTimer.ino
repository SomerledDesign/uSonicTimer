/**
 * @file uSonicTimer.ino
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief an addition to an old, inexpensive Ultrasonic cleaner to include heating and timing
 * @version 0.2
 * @date 01/29/22
 *
 * @copyright Copyright (c) 2022 Somerled Design, LLC in Kevin Murphy
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
 *
 *
 * DISCLAIMER:
 *   With this design, including both the hardware & software I offer no guarantee that it is bug
 *   free or reliable. So, if you decide to build one and you have problems or issues and/or causes
 *   damage/harm to you, others or property then you are on your own. This work is experimental.
 *
 */

/* Temp sensor includes */
#include <DallasTemperature.h>

/* for display */
#include <U8g2lib.h>

/* for rotary encoder */
#include <Rotary.h>

/* for timer */
#include <TimerOne.h>

/* for buzzer */
#include <toneAC.h>

/* DEBUG macros */
#define DEBUG 1

#if DEBUG == 1
#define beginDebug(x) Serial.begin(x)
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define beginDebug(x)
#define debug(x)
#define debugln(x)
#endif // DEBUG

// How big is Your display?
#define WIDTH 128
#define HEIGHT 64

// added to reduce PROGMEM usage.  Only include the splash you need :^)
#if HEIGHT == 32
#include "splash32.h"
#else
#include "splash64.h"
#endif

// #define SCREEN_ADDRESS 0x3C // YES, I know what it says on the back.
// This is what I2C Scanner returns
// It must be 7bit not 8bit
// is this even needed for u8g2lib?

long seconds = 136; // just a random seconds to start from for testing

uint16_t hours;
uint8_t mins;
uint8_t secs;

uint16_t *h = &hours;
uint8_t *m = &mins;
uint8_t *s = &secs;

// Temp sensor data bus
#define ONE_WIRE_BUS 14 // A0
//                                   0x28, 0xFF, 0x57, 0x3F, 0x01, 0x16, 0x01, 0xED
DeviceAddress thermometerAddress = {0x28, 0xFF, 0x57, 0x3F, 0x01, 0x16, 0x01, 0xED}; // custom array type to hold 64 bit device address

// Rotary encoder with switch
#define ROTARY_PIN1 2
#define ROTARY_PIN2 3
#define ROTARY_SWITCH 4
#define ROTARY_ACCEL_OFFSET1 20
#define ROTARY_ACCEL_OFFSET2 50
#define ROTARY_ACCEL_OFFSET3 70
unsigned long rotaryLastMove;
bool rotaryButtonPressed = false;

// Timer
#define TIMER 1000
// Timer settings
// in seconds
#define THREE_MINUTES 180
#define EIGHT_MINUTES 480
#define TEN_MINUTES 600
#define FIFTEEN_MINUTES 900
#define TWENTY_MINUTES 1200
#define THIRTY_MINUTES 1800
#define SIXTY_MINUTES 3600
#define NINETY_MINUTES 5400

// Status LED
#define STATUS_LED_PIN 9
bool statusLedOn = false;

// BUZZER is pins 9 & 10 for toneAC

/**
 * Initialize a new display instance for a display connected to I2C SLC and SDA.
 *
 *  Rotation protocols
 *
 * _Rotation/Mirror_              _Description_
 *  U8G2_R0               No rotation, landscape
 *  U8G2_R1               90 degree clockwise rotation
 *  U8G2_R2               180 degree clockwise rotation
 *  U8G2_R3               270 degree clockwise rotation
 *  U8G2_MIRROR           No rotation, landscape, display content is mirrored (v2.6.x)
 *  U8G2_MIRROR_VERTICAL  No rotation, landscape, display content is mirrored and flipped verticaly
 *
 */

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

// temperature sensing stuff
OneWire oneWire(ONE_WIRE_BUS);          // create a oneWire instance to communicate with temperature IC
DallasTemperature tempSensor(&oneWire); // pass the oneWire reference to Dallas Temperature sensor

/**
 * @author guix
 * @link https://forum.arduino.cc/t/convert-seconds-variable-into-hours-minutes-seconds/273112 @endlink
 * @brief
 *
 * @param seconds - seconds to convert
 * @param h - ref to hours
 * @param m - ref to mins
 * @param s - ref to secs
 *
 */
void secondsToHMS(const uint32_t seconds, uint16_t &h, uint8_t &m, uint8_t &s)
{
  uint32_t t = seconds;
  // remainder seconds
  s = t % 60;
  // remainder minutes
  t = (t - s) / 60;
  m = t % 60;
  // remainder hours
  t = (t - m) / 60;
  h = t;
}

// ===============================================
// SETUP         SETUP         SETUP         SETUP
// ===============================================

void setup()
{
  // put your setup code here, to run once:
  beginDebug(9600);

  u8g2.begin();

  // begin splash screen code
  u8g2.firstPage();
  do
  {
    u8g2.drawXBMP(
        (WIDTH - splash1_width) / 2,   // x pos
        (HEIGHT - splash1_height) / 2, // y pos
        splash1_width,                 // width
        splash1_height,                // height
        splash1_bits);                 // *bitmap

  } while (u8g2.nextPage());
  delay(2000);
  u8g2.clearDisplay();
  do
  {
    u8g2.drawXBMP(
        (WIDTH - splash2_width) / 2,   // x pos
        (HEIGHT - splash2_height) / 2, // y pos
        splash2_width,                 // width
        splash2_height,                // height
        splash2_bits);                 // *bitmap
  } while (u8g2.nextPage());
  delay(4000);
  // end splash screen code
}

// ======================================================
//   LOOP        LOOP        LOOP       LOOP       LOOP
// ======================================================
void loop()
{
  // put your main code here, to run repeatedly:
  u8g2.firstPage();
  do
  {
    /* all graphics commands have to appear within the loop body. */
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.drawStr(0, 16, "Hello World!");
  } while (u8g2.nextPage());
}
