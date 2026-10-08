/**
 * Host-side stand-in for U8g2lib.h: just enough of the U8G2 C++ class, on top of the real
 * U8g2 C library (csrc), for screens.cpp to draw into an 84x48 PCD8544 frame buffer on a PC.
 */
#pragma once

#include <stdint.h>
#include "u8g2.h"

#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#endif

class U8G2
{
public:
    U8G2();
    u8g2_t *getU8g2() { return &u8g2; }
    void clearBuffer() { u8g2_ClearBuffer(&u8g2); }
    void setFont(const uint8_t *font) { u8g2_SetFont(&u8g2, font); }
    void setFontMode(uint8_t mode) { u8g2_SetFontMode(&u8g2, mode); }
    void setDrawColor(uint8_t color) { u8g2_SetDrawColor(&u8g2, color); }
    void drawPixel(u8g2_uint_t x, u8g2_uint_t y) { u8g2_DrawPixel(&u8g2, x, y); }
    void drawHLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w) { u8g2_DrawHLine(&u8g2, x, y, w); }
    void drawVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t h) { u8g2_DrawVLine(&u8g2, x, y, h); }
    void drawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) { u8g2_DrawBox(&u8g2, x, y, w, h); }
    void drawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) { u8g2_DrawFrame(&u8g2, x, y, w, h); }
    u8g2_uint_t drawStr(u8g2_uint_t x, u8g2_uint_t y, const char *s) { return u8g2_DrawStr(&u8g2, x, y, s); }
    u8g2_uint_t drawUTF8(u8g2_uint_t x, u8g2_uint_t y, const char *s) { return u8g2_DrawUTF8(&u8g2, x, y, s); }
    u8g2_uint_t getStrWidth(const char *s) { return u8g2_GetStrWidth(&u8g2, s); }
    u8g2_uint_t getUTF8Width(const char *s) { return u8g2_GetUTF8Width(&u8g2, s); }
    int8_t getAscent() { return u8g2_GetAscent(&u8g2); }
    int8_t getDescent() { return u8g2_GetDescent(&u8g2); }
    uint8_t *getBufferPtr() { return u8g2_GetBufferPtr(&u8g2); }
    uint8_t getBufferTileWidth() { return u8g2_GetBufferTileWidth(&u8g2); }
    bool getPixel(int x, int y); // true = black

private:
    u8g2_t u8g2;
};
