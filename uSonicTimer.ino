// Arduino Countdown Timer Code

/* Display includes */
// #include <LiquidCrystal.h>
// #include <Wire.h.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

/* Buzzer includes */
#include <toneAC.h>

/* DEBUG macros */
#define DEBUG 1
// #define SSD1306_NO_SPLASH

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

#if HEIGHT == 32
#include "splash32.h"
#else
#include "splash64.h"
#endif

#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C // YES, I know what it says on the back.  This is what I2C Scanner returns

/***
 *      ___           _              _   _       _   _
 *     |_ _|_ __  ___| |_ __ _ _ __ | |_(_) __ _| |_(_) ___  _ __  ___
 *      | || '_ \/ __| __/ _` | '_ \| __| |/ _` | __| |/ _ \| '_ \/ __|
 *      | || | | \__ | || (_| | | | | |_| | (_| | |_| | (_) | | | \__ \
 *     |___|_| |_|___/\__\__,_|_| |_|\__|_|\__,_|\__|_|\___/|_| |_|___/
 *
 */
Adafruit_SSD1306 oled(WIDTH, HEIGHT, &Wire, OLED_RESET);

#define THREE_MINUTES 3 * 60
#define EIGHT_MINUTES 8 * 60
#define TEN_MINUTES 10 * 60
#define FIFTEEN_MINUTES 15 * 60
#define THIRTY_MINUTES 30 * 60
#define SIXTY_MINUTES 60 * 60
#define NINETY_MINUTES 90 * 60

boolean feed = true; // condition for alarm

char key;
String r[8];

void splash(void)
{ 
  // debugln("...entered splash();");

  oled.drawBitmap(
    (WIDTH - splash1_width) / 2, 
    (HEIGHT - splash1_height) / 2,
    splash1_data, 
    splash1_width, 
    splash1_height, 
    1
    );

  oled.display();

  debugln("...after first display(); in splash();");
  delay(1800);
  oled.clearDisplay();

  oled.drawBitmap(
      (WIDTH - splash2_width) / 2,
      (HEIGHT - splash2_height) / 2,
      splash2_data,
      splash2_width,
      splash2_height,
      1);

  oled.display();

  // debugln("...after second display(); in splash();");
  
  delay(3000);
  
  oled.clearDisplay();

} // end splash

// ===============================================
// SETUP  SETUP  SETUP  SETUP  SETUP  SETUP  SETUP
// ===============================================

void setup()
{
  // put your setup code here, to run once:
  beginDebug(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  oled.display();
  oled.clearDisplay();

  splash();
  oled.display();
  debugln("\tSetup complete...");
} // end setup

// =============================================
// LOOP  LOOP  LOOP  LOOP  LOOP LOOP  LOOP  LOOP
// =============================================

void loop()
{
  // put your main code here, to run repeatedly
  debugln("...entered loop...");
  delay(1200);
} // end loop