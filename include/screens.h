/**
 * @file screens.h
 * @brief Drawing for the uSonicTimer pages (Nokia 5110, 84x48).
 *
 * These functions only draw into the U8g2 frame buffer; the caller clears nothing beforehand
 * (each one starts with clearBuffer()) and sends the buffer with sendBufferPolled() afterwards.
 * They take plain values rather than reading globals so tools/host/ can render them on a PC.
 */
#pragma once

#include <U8g2lib.h>
#include <stdint.h>

#define LCD_WIDTH  84
#define LCD_HEIGHT 48

// Degree sign: code 0xB0 in the Latin-1 range of the _tf fonts, drawn with drawStr().
// Keep it a separate string literal ("\xB0" "F"), otherwise the F joins the hex escape.
#define DEG_SIGN "\xB0"

// ---- big characters (blocky font in bigfont.h) ----
#define BIG_CHAR_W    15
#define BIG_CHAR_H    28
#define BIG_COLON_W   4  // the colon is two 4x4 blocks
#define BIG_GAP       3  // between characters
#define BIG_TOP       2  // top row of the big characters on the run screens

uint8_t bigTextWidth(const char *s);
/// draws s at (x, y) = top-left; colonOn = false leaves the colon's space blank. Returns the width.
uint8_t drawBigText(U8G2 &g, int16_t x, int16_t y, const char *s, bool colonOn = true);

// ---- run screens (big characters, progress rule, one line of small text, bottom rule) ----
/// "MM:SS" (max 60:00); progress = filled pixels of the rule under the digits (0..84)
void drawTimerScreen(U8G2 &g, uint16_t remainingS, bool colonOn, uint8_t progress, const char *line);
/// whole degrees in the display unit with a small unit in the bottom-right corner
void drawHeatingScreen(U8G2 &g, int16_t temp, bool celsius, uint8_t progress, const char *line);
void drawDoneScreen(U8G2 &g, const char *line);
/// text for the timer screen: "Now 118 Set 120°F" (or "NO SENSOR"), fitted to 84 px
void formatTimerLine(U8G2 &g, char *buf, uint8_t size, bool sensorOk, int16_t now, int16_t set, bool celsius);

// ---- menus and settings pages ----
void drawTitleBar(U8G2 &g, const char *title);
/// list with a title bar; firstRow is the scroll position, kept by the caller between frames
/// footer (optional, may be nullptr) is a line of small text between two rules under the items
void drawMenuList(U8G2 &g, const char *title, const char *const *items, uint8_t count,
                  uint8_t selected, uint8_t &firstRow, const char *footer);
/// digit editor (Set temp): n digits, cursor digit in reverse video, then suffix (e.g. "°F")
void drawDigitEditor(U8G2 &g, const char *title, const uint8_t *digits, uint8_t n, uint8_t cursor,
                     const char *suffix, const char *hint);
/// one value line, an optional level bar (barMax 0 = none) and a hint line
void drawValuePage(U8G2 &g, const char *title, const char *value, uint8_t barLevel, uint8_t barMax,
                   const char *hint);
