/**
 * @file bigfont.h
 * @brief Glyphs for the big-digit screens, copied from the classic Adafruit 5x7 font.
 *
 * Source: extras/glcdfont.c (Adafruit GFX "Standard ASCII 5x7 font", BSD licence), only the
 * characters the big screens need. 5 bytes per character, one byte per column (left to right),
 * bit 0 = top row, 7 rows used. The zero is the slashed one from that font.
 * screens.cpp draws them scaled 3x wide and 4x tall (15x28 px per character).
 */
#pragma once

#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h> // PROGMEM, pgm_read_byte
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#endif
#endif

#define BIG_GLYPH_COLS 5
#define BIG_GLYPH_ROWS 7

// characters in BIG_GLYPHS, same order
static const char BIG_GLYPH_CHARS[] = "-0123456789:DEON";

static const uint8_t BIG_GLYPHS[][BIG_GLYPH_COLS] PROGMEM = {
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0' (slashed)
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x72, 0x49, 0x49, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x49, 0x4D, 0x33}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x31}, // '6'
    {0x41, 0x21, 0x11, 0x09, 0x07}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x46, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x00, 0x14, 0x00, 0x00}, // ':' (drawn narrow: only its lit column)
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
};

static_assert(sizeof(BIG_GLYPHS) / sizeof(BIG_GLYPHS[0]) == sizeof(BIG_GLYPH_CHARS) - 1,
              "BIG_GLYPHS and BIG_GLYPH_CHARS differ in length");
