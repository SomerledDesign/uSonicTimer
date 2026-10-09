// Renders the uSonicTimer screens with the real drawing code (src/screens.cpp) and the real
// U8g2 C library into 84x48 PBM files; render.py turns them into scaled-up PNGs.
#include "screens.h"
#include "ust_logic.h"

#include <stdio.h>
#include <string.h>

static void save(U8G2 &g, const char *dir, const char *name)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.pbm", dir, name);
    FILE *f = fopen(path, "w");
    if (!f)
    {
        perror(path);
        return;
    }
    fprintf(f, "P1\n%d %d\n", LCD_WIDTH, LCD_HEIGHT);
    for (int y = 0; y < LCD_HEIGHT; y++)
    {
        for (int x = 0; x < LCD_WIDTH; x++)
        {
            fputc(g.getPixel(x, y) ? '1' : '0', f);
        }
        fputc('\n', f);
    }
    fclose(f);
    printf("%s\n", path);
}

int main(int argc, char **argv)
{
    const char *dir = (argc > 1) ? argv[1] : ".";
    U8G2 g;
    g.setFontMode(1);
    char line[24];

    // timer: 10 min run, 23 s left would be "00:23"; show 07:42 of 10:00, colon on and off
    {
        const uint32_t total = 600000, elapsed = 600000 - 462000;
        formatTimerLine(g, line, sizeof(line), true, displayTemp(118.4f, false), 120, false);
        drawTimerScreen(g, remainingSeconds(total, elapsed), true, progressPx(elapsed, total, LCD_WIDTH), line);
        save(g, dir, "timer_F");
        drawTimerScreen(g, remainingSeconds(total, elapsed), false, progressPx(elapsed, total, LCD_WIDTH), line);
        save(g, dir, "timer_F_colon_off");
        formatTimerLine(g, line, sizeof(line), true, displayTemp(118.4f, true), displayTemp(120, true), true);
        drawTimerScreen(g, 23, true, progressPx(577000, 600000, LCD_WIDTH), line);
        save(g, dir, "timer_C");
        formatTimerLine(g, line, sizeof(line), true, displayTemp(-40.0f, false), 180, false);
        drawTimerScreen(g, 3600, true, 0, line);
        save(g, dir, "timer_worst_case");
        formatTimerLine(g, line, sizeof(line), true, displayTemp(118.4f, false), 120, false);
        drawTimerScreen(g, 12 * 60 + 48, true, progressPx(220000, 600000, LCD_WIDTH), line);
        save(g, dir, "timer_12_48");
        formatTimerLine(g, line, sizeof(line), false, 0, 120, false);
        drawTimerScreen(g, 245, true, progressPx(355000, 600000, LCD_WIDTH), line);
        save(g, dir, "timer_no_sensor");
    }
    // heating: started at 72 F, now 98.6 F, set 120 F
    {
        snprintf(line, sizeof(line), "Heating to %d" DEG_SIGN "F", 120);
        drawHeatingScreen(g, displayTemp(98.6f, false), false, heatProgressPx(72.0f, 98.6f, 120.0f, LCD_WIDTH), line);
        save(g, dir, "heating_F");
        snprintf(line, sizeof(line), "Heating to %d" DEG_SIGN "C", displayTemp(120, true));
        drawHeatingScreen(g, displayTemp(98.6f, true), true, heatProgressPx(72.0f, 98.6f, 120.0f, LCD_WIDTH), line);
        save(g, dir, "heating_C");
        snprintf(line, sizeof(line), "Heating to %d" DEG_SIGN "F", 180);
        drawHeatingScreen(g, displayTemp(-5.2f, false), false, heatProgressPx(-5.2f, -5.2f, 180.0f, LCD_WIDTH), line);
        save(g, dir, "heating_negative");
    }
    // done
    {
        snprintf(line, sizeof(line), "Cleaned %u min", 10u);
        drawDoneScreen(g, line);
        save(g, dir, "done");
    }
    // menus
    {
        static const char *const mainItems[] = {"Start", "Settings..."};
        uint8_t first = 0;
        snprintf(line, sizeof(line), "%d" DEG_SIGN "F  %u min", 120, 10u);
        drawMenuList(g, "uSonicTimer", mainItems, 2, 0, first, line);
        save(g, dir, "menu_main");
        static const char *const settingsItems[] = {"Set temp", "Set time", "Set units", "Set backlight", "Set contrast", "Exit"};
        first = 0;
        drawMenuList(g, "Settings", settingsItems, 6, 0, first, nullptr);
        save(g, dir, "menu_settings_top");
        drawMenuList(g, "Settings", settingsItems, 6, 3, first, nullptr);
        drawMenuList(g, "Settings", settingsItems, 6, 4, first, nullptr);
        save(g, dir, "menu_settings_scrolled");
    }
    // settings pages
    {
        const uint8_t d3[3] = {1, 2, 0};
        drawDigitEditor(g, "Set temp", d3, 3, 1, DEG_SIGN "F", "Press = next");
        save(g, dir, "page_set_temp_F");
        drawDigitEditor(g, "Set temp", d3, 3, 2, DEG_SIGN "F", "Press = save");
        save(g, dir, "page_set_temp_F_last_digit");
        const uint8_t d2[2] = {4, 9};
        drawDigitEditor(g, "Set temp", d2, 2, 0, DEG_SIGN "C", "Press = next");
        save(g, dir, "page_set_temp_C");
        drawValuePage(g, "Set time", "10 min", 0, 0, "Press = save");
        save(g, dir, "page_set_time");
        drawValuePage(g, "Set units", DEG_SIGN "F Fahrenheit", 0, 0, "Press = save");
        save(g, dir, "page_set_units");
        drawValuePage(g, "Set backlight", "Level 7", 7, 10, "Press = save");
        save(g, dir, "page_set_backlight");
        drawValuePage(g, "Set contrast", "52", 52 - CONTRAST_UI_MIN, CONTRAST_UI_MAX - CONTRAST_UI_MIN, "Press = save");
        save(g, dir, "page_set_contrast");
    }
    return 0;
}
