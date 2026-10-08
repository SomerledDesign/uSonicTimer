/**
 * @file screens.cpp
 * @brief Drawing for the uSonicTimer pages, see screens.h.
 *
 * Run screens follow the cbm80amiga Nokia 5110 clock style: big chunky characters filling the
 * top of the screen, a 1 px rule under them that doubles as a progress bar, one line of small
 * text and another rule. Menus and settings pages get a solid title bar with inverse text.
 *
 * Layout (y = pixel rows, 0 = top):
 *   run screens   big characters 2..29, progress rule 33 (fill 32..34), text caps 37..42
 *                 (baseline 43), bottom rule 46
 *   menus/pages   title bar 0..7 (5x7 font, inverse), list rows of 10 px from y = 8
 */
#include "screens.h"
#include "bigfont.h"

#include <stdio.h>
#include <string.h>

// small fonts (u8g2 X11 misc-fixed, _tf = with Latin-1, so the degree sign is there)
#define FONT_SMALL u8g2_font_5x7_tf  // 5 px per char, 16 per line; caps 6 px
#define FONT_TEXT  u8g2_font_6x10_tf // 6 px per char, 14 per line; caps 7 px

// run-screen rows
#define RUN_RULE_Y     33 // progress rule (1 px); the filled part is 3 px tall
#define RUN_TEXT_BASE  43
#define RUN_BOTTOM_Y   46

// menu rows
#define TITLE_H        8
#define TITLE_BASE     7
#define ROW_H          10
#define MENU_ROWS      4 // (48 - 8) / 10

// ---------------------------------------------------------------------------------
// big characters
// ---------------------------------------------------------------------------------
static int8_t bigGlyphIndex(char c)
{
    const char *p = strchr(BIG_GLYPH_CHARS, c);
    return (p != nullptr && c != '\0') ? static_cast<int8_t>(p - BIG_GLYPH_CHARS) : -1;
}

static uint8_t bigCharWidth(char c)
{
    return (c == ':') ? BIG_COLON_W : BIG_CHAR_W;
}

uint8_t bigTextWidth(const char *s)
{
    uint16_t w = 0;
    for (const char *p = s; *p != '\0'; p++)
    {
        w += bigCharWidth(*p) + ((p != s) ? BIG_GAP : 0);
    }
    return static_cast<uint8_t>((w > 255) ? 255 : w);
}

/// one character; unknown characters (and ' ') are blank
static void drawBigChar(U8G2 &g, int16_t x, int16_t y, char c)
{
    const int8_t idx = bigGlyphIndex(c);
    if (idx < 0)
    {
        return;
    }
    uint8_t firstCol = 0;
    uint8_t lastCol = BIG_GLYPH_COLS - 1;
    if (c == ':') // narrow colon: only its lit column(s)
    {
        firstCol = 2;
        lastCol = 2;
    }
    for (uint8_t col = firstCol; col <= lastCol; col++)
    {
        const uint8_t bits = pgm_read_byte(&BIG_GLYPHS[idx][col]);
        const int16_t px = x + (col - firstCol) * BIG_SCALE_X;
        // one box per vertical run of set pixels
        uint8_t row = 0;
        while (row < BIG_GLYPH_ROWS)
        {
            if (!(bits & (1 << row)))
            {
                row++;
                continue;
            }
            uint8_t end = row;
            while (end < BIG_GLYPH_ROWS && (bits & (1 << end)))
            {
                end++;
            }
            const int16_t py = y + row * BIG_SCALE_Y;
            if (px >= 0 && py >= 0 && px + BIG_SCALE_X <= LCD_WIDTH && py + (end - row) * BIG_SCALE_Y <= LCD_HEIGHT)
            {
                g.drawBox(px, py, BIG_SCALE_X, (end - row) * BIG_SCALE_Y);
            }
            row = end;
        }
    }
}

uint8_t drawBigText(U8G2 &g, int16_t x, int16_t y, const char *s, bool colonOn)
{
    const int16_t x0 = x;
    for (const char *p = s; *p != '\0'; p++)
    {
        if (p != s)
        {
            x += BIG_GAP;
        }
        if (*p != ':' || colonOn)
        {
            drawBigChar(g, x, y, *p);
        }
        x += bigCharWidth(*p);
    }
    return static_cast<uint8_t>(x - x0);
}

// ---------------------------------------------------------------------------------
// run screens
// ---------------------------------------------------------------------------------
static void drawCentredStr(U8G2 &g, int16_t baseline, const char *s)
{
    const int16_t w = g.getStrWidth(s);
    const int16_t x = (LCD_WIDTH - w) / 2;
    g.drawStr((x < 0) ? 0 : x, baseline, s);
}

/// progress rule under the big characters, the small text line and the bottom rule
static void drawRunFooter(U8G2 &g, uint8_t progress, const char *line)
{
    if (progress > LCD_WIDTH)
    {
        progress = LCD_WIDTH;
    }
    g.setDrawColor(1);
    g.drawHLine(0, RUN_RULE_Y, LCD_WIDTH);
    if (progress > 0)
    {
        g.drawBox(0, RUN_RULE_Y - 1, progress, 3);
    }
    g.setFont(FONT_SMALL);
    drawCentredStr(g, RUN_TEXT_BASE, line);
    g.drawHLine(0, RUN_BOTTOM_Y, LCD_WIDTH);
}

void drawTimerScreen(U8G2 &g, uint16_t remainingS, bool colonOn, uint8_t progress, const char *line)
{
    if (remainingS > 3600)
    {
        remainingS = 3600; // 60:00 is the longest run
    }
    char buf[8];
    snprintf(buf, sizeof(buf), "%02u:%02u", static_cast<unsigned>(remainingS / 60), static_cast<unsigned>(remainingS % 60));
    g.clearBuffer();
    g.setDrawColor(1);
    drawBigText(g, (LCD_WIDTH - bigTextWidth(buf)) / 2, BIG_TOP, buf, colonOn);
    drawRunFooter(g, progress, line);
}

void drawHeatingScreen(U8G2 &g, int16_t temp, bool celsius, uint8_t progress, const char *line)
{
    if (temp < -99)
    {
        temp = -99;
    }
    if (temp > 999)
    {
        temp = 999;
    }
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", temp);
    const char *unit = celsius ? DEG_SIGN "C" : DEG_SIGN "F";

    g.clearBuffer();
    g.setDrawColor(1);
    g.setFont(FONT_TEXT);
    const int16_t unitW = g.getStrWidth(unit);
    const int16_t unitX = LCD_WIDTH - unitW;
    // big digits right-aligned against the unit; the unit sits on the digits' bottom row
    int16_t x = unitX - 2 - bigTextWidth(buf);
    if (x < 0)
    {
        x = 0;
    }
    drawBigText(g, x, BIG_TOP, buf);
    g.drawStr(unitX, BIG_TOP + BIG_CHAR_H, unit);
    drawRunFooter(g, progress, line);
}

void drawDoneScreen(U8G2 &g, const char *line)
{
    g.clearBuffer();
    g.setDrawColor(1);
    drawBigText(g, (LCD_WIDTH - bigTextWidth("DONE")) / 2, BIG_TOP, "DONE");
    drawRunFooter(g, LCD_WIDTH, line);
}

void formatTimerLine(U8G2 &g, char *buf, uint8_t size, bool sensorOk, int16_t now, int16_t set, bool celsius)
{
    if (!sensorOk)
    {
        snprintf(buf, size, "NO SENSOR");
        return;
    }
    const char *unit = celsius ? DEG_SIGN "C" : DEG_SIGN "F";
    g.setFont(FONT_SMALL);
    snprintf(buf, size, "Now %d  Set %d%s", now, set, unit);
    if (g.getStrWidth(buf) <= LCD_WIDTH)
    {
        return;
    }
    snprintf(buf, size, "Now %d Set %d%s", now, set, unit); // 17 chars = 84 px at most
    if (g.getStrWidth(buf) <= LCD_WIDTH)
    {
        return;
    }
    snprintf(buf, size, "%d / %d%s", now, set, unit);
}

// ---------------------------------------------------------------------------------
// menus and settings pages
// ---------------------------------------------------------------------------------
void drawTitleBar(U8G2 &g, const char *title)
{
    g.setDrawColor(1);
    g.drawBox(0, 0, LCD_WIDTH, TITLE_H);
    g.setFont(FONT_SMALL);
    g.setDrawColor(0); // inverse text (font mode 1: only the strokes are cleared)
    drawCentredStr(g, TITLE_BASE, title);
    g.setDrawColor(1);
}

static void drawHint(U8G2 &g, const char *hint)
{
    if (hint == nullptr || hint[0] == '\0')
    {
        return;
    }
    g.setDrawColor(1);
    g.drawHLine(0, 38, LCD_WIDTH);
    g.setFont(FONT_SMALL);
    drawCentredStr(g, 47, hint);
}

void drawMenuList(U8G2 &g, const char *title, const char *const *items, uint8_t count,
                  uint8_t selected, uint8_t &firstRow, const char *footer)
{
    // rows available under the title bar (a footer takes the bottom of the screen)
    const uint8_t rows = (footer != nullptr) ? 2 : MENU_ROWS;
    if (selected >= count)
    {
        selected = count - 1;
    }
    if (selected < firstRow)
    {
        firstRow = selected;
    }
    else if (selected >= firstRow + rows)
    {
        firstRow = selected - rows + 1;
    }
    if (firstRow + rows > count)
    {
        firstRow = (count > rows) ? count - rows : 0;
    }

    g.clearBuffer();
    drawTitleBar(g, title);
    g.setFont(FONT_TEXT);
    for (uint8_t row = 0; row < rows && (firstRow + row) < count; row++)
    {
        const uint8_t i = firstRow + row;
        const int16_t top = TITLE_H + row * ROW_H;
        g.setDrawColor(1);
        if (i == selected)
        {
            g.drawBox(0, top, LCD_WIDTH, ROW_H);
            g.setDrawColor(0);
        }
        g.drawStr(1, top + 8, items[i]); // 13 chars end at x 78, clear of the scroll arrows
        g.setDrawColor(1);
    }
    // more items above/below: small arrows at the right edge
    if (count > rows)
    {
        for (uint8_t k = 0; k < 3; k++)
        {
            if (firstRow > 0)
            {
                g.setDrawColor((selected == firstRow) ? 0 : 1);
                g.drawHLine(LCD_WIDTH - 3 - k, TITLE_H + 2 + k, 1 + 2 * k);
            }
            if (firstRow + rows < count)
            {
                g.setDrawColor((selected == firstRow + rows - 1) ? 0 : 1);
                g.drawHLine(LCD_WIDTH - 3 - k, TITLE_H + rows * ROW_H - 3 - k, 1 + 2 * k);
            }
        }
        g.setDrawColor(1);
    }
    if (footer != nullptr)
    {
        // same look as the run screens: rule, one line of small text, rule
        g.drawHLine(0, RUN_RULE_Y, LCD_WIDTH);
        g.setFont(FONT_SMALL);
        drawCentredStr(g, RUN_TEXT_BASE, footer);
        g.drawHLine(0, RUN_BOTTOM_Y, LCD_WIDTH);
    }
}

void drawDigitEditor(U8G2 &g, const char *title, const uint8_t *digits, uint8_t n, uint8_t cursor,
                     const char *suffix, const char *hint)
{
    g.clearBuffer();
    drawTitleBar(g, title);
    g.setFont(FONT_TEXT);
    const int16_t baseline = 26;
    const int16_t suffixW = g.getStrWidth(suffix);
    int16_t x = (LCD_WIDTH - (n * 10 + 2 + suffixW)) / 2;
    if (x < 0)
    {
        x = 0;
    }
    for (uint8_t i = 0; i < n; i++)
    {
        g.setDrawColor(1);
        if (i == cursor)
        {
            g.drawBox(x + i * 10, baseline - 8, 10, 10);
            g.setDrawColor(0);
        }
        char d[2] = {static_cast<char>('0' + (digits[i] % 10)), '\0'};
        g.drawStr(x + i * 10 + 2, baseline, d); // +2 centres the 6 px glyph in the 10 px box
        g.setDrawColor(1);
    }
    g.drawStr(x + n * 10 + 2, baseline, suffix);
    drawHint(g, hint);
}

void drawValuePage(U8G2 &g, const char *title, const char *value, uint8_t barLevel, uint8_t barMax,
                   const char *hint)
{
    g.clearBuffer();
    drawTitleBar(g, title);
    g.setFont(FONT_TEXT);
    const int16_t baseline = (barMax > 0) ? 21 : 26;
    drawCentredStr(g, baseline, value);
    if (barMax > 0)
    {
        // framed bar, 72 px inside
        const int16_t barX = 5;
        const int16_t barW = LCD_WIDTH - 2 * barX;
        g.drawFrame(barX, 26, barW, 7);
        if (barLevel > barMax)
        {
            barLevel = barMax;
        }
        const int16_t fill = static_cast<int16_t>((static_cast<uint16_t>(barW - 4) * barLevel) / barMax);
        if (fill > 0)
        {
            g.drawBox(barX + 2, 28, fill, 3);
        }
    }
    drawHint(g, hint);
}
