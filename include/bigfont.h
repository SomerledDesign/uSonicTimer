/**
 * @file bigfont.h
 * @brief Big chunky characters for the run screens (timer, heating, DONE).
 *
 * Squared-off, 7-segment-like digits in the style of the cbm80amiga Nokia 5110 clock: thick
 * strokes, 90 degree corners, no diagonals except the stepped slash of the zero. Cell 15x28 px
 * (the colon is 4 px wide). The upper half (rows 0-11) is inset 1 px on each side with 4 px
 * verticals; from the middle bar (rows 12-15) down the glyphs are full width with 5 px
 * verticals, which gives the stepped silhouette of the reference font. Horizontal bars are
 * 4 px tall.
 *
 * Each character is a list of filled boxes (x, y, w, h relative to the cell's top-left);
 * screens.cpp draws them with drawBox(). Edit the boxes directly; tools/host/test_logic.cpp
 * checks that they stay inside the cell (BIG_CELL_W x BIG_CELL_H, the colon BIG_COLON_W wide).
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

#define BIG_CELL_W 15
#define BIG_CELL_H 28

struct BigBox
{
    uint8_t x, y, w, h;
};

// characters in the font, same order as BIG_GLYPH_FIRST
static const char BIG_GLYPH_CHARS[] = "-0123456789:DEON";

static const BigBox BIG_BOXES[] PROGMEM = {
    // minus
    {2, 12, 11, 4},
    // slashed zero (stepped slash)
    {1, 0, 13, 4}, {1, 4, 4, 8}, {10, 4, 4, 8}, {0, 12, 5, 4}, {10, 12, 5, 4}, {0, 16, 5, 8}, {10, 16, 5, 8}, {0, 24, 15, 4}, {7, 9, 2, 5}, {6, 14, 2, 5},
    // '1'
    {7, 0, 4, 8}, {5, 8, 6, 20},
    // '2'
    {1, 0, 13, 4}, {1, 4, 4, 4}, {10, 4, 4, 8}, {0, 12, 15, 4}, {0, 16, 5, 8}, {0, 24, 15, 4},
    // '3'
    {1, 0, 13, 4}, {10, 4, 4, 8}, {4, 12, 11, 4}, {10, 16, 5, 8}, {0, 24, 15, 4},
    // '4'
    {1, 0, 4, 12}, {10, 0, 4, 12}, {0, 12, 15, 4}, {10, 16, 5, 12},
    // '5'
    {1, 0, 13, 4}, {1, 4, 4, 8}, {0, 12, 15, 4}, {10, 16, 5, 8}, {0, 20, 5, 4}, {0, 24, 15, 4},
    // '6'
    {1, 0, 13, 4}, {1, 4, 4, 8}, {0, 12, 15, 4}, {0, 16, 5, 8}, {10, 16, 5, 8}, {0, 24, 15, 4},
    // '7'
    {1, 0, 13, 4}, {10, 4, 4, 8}, {10, 12, 5, 16},
    // '8'
    {1, 0, 13, 4}, {1, 4, 4, 8}, {10, 4, 4, 8}, {0, 12, 15, 4}, {0, 16, 5, 8}, {10, 16, 5, 8}, {0, 24, 15, 4},
    // '9'
    {1, 0, 13, 4}, {1, 4, 4, 8}, {10, 4, 4, 8}, {0, 12, 15, 4}, {10, 16, 5, 8}, {0, 24, 15, 4},
    // colon (4 px wide cell)
    {0, 7, 4, 4}, {0, 17, 4, 4},
    // 'D'
    {0, 0, 11, 4}, {0, 4, 5, 20}, {10, 4, 5, 20}, {0, 24, 11, 4},
    // 'E'
    {0, 0, 15, 4}, {0, 4, 5, 20}, {0, 12, 12, 4}, {0, 24, 15, 4},
    // 'O'
    {1, 0, 13, 4}, {0, 4, 5, 20}, {10, 4, 5, 20}, {1, 24, 13, 4},
    // 'N'
    {0, 0, 5, 28}, {10, 0, 5, 28}, {5, 3, 2, 6}, {6, 8, 2, 6}, {7, 13, 2, 6}, {8, 18, 2, 6},
};

// index of each character's first box in BIG_BOXES; the next entry ends the list
static const uint8_t BIG_GLYPH_FIRST[] PROGMEM = {0, 1, 11, 13, 19, 24, 28, 34, 40, 43, 50, 56, 58, 62, 66, 70, 76};

static_assert(sizeof(BIG_GLYPH_FIRST) == sizeof(BIG_GLYPH_CHARS), "BIG_GLYPH_FIRST needs one entry per character plus one");
static_assert(sizeof(BIG_BOXES) / sizeof(BIG_BOXES[0]) == 76, "BIG_BOXES and BIG_GLYPH_FIRST differ");
