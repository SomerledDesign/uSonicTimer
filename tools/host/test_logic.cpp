// Host-side checks for include/ust_logic.h and the big-font layout (see build.sh).
#include "ust_logic.h"
#include "screens.h"
#include "bigfont.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;
#define CHECK(cond)                                                    \
    do                                                                 \
    {                                                                  \
        if (!(cond))                                                   \
        {                                                              \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);     \
            failures++;                                                \
        }                                                              \
    } while (0)

int main()
{
    // ---- hysteresis: heater below set-10, resume at set-8 ----
    const float set = 120.0f;
    CHECK(nextRunState(RUN_START, true, 70.0f, set) == RUN_HEATING);
    CHECK(nextRunState(RUN_START, true, 110.0f, set) == RUN_CLEANING);  // exactly set-10: no heat
    CHECK(nextRunState(RUN_START, true, 109.9f, set) == RUN_HEATING);
    CHECK(nextRunState(RUN_START, true, 111.0f, set) == RUN_CLEANING);  // in the gap: within 10 F
    CHECK(nextRunState(RUN_HEATING, true, 111.9f, set) == RUN_HEATING); // in the gap: keep heating
    CHECK(nextRunState(RUN_HEATING, true, 112.0f, set) == RUN_CLEANING); // set-8 reached
    CHECK(nextRunState(RUN_CLEANING, true, 110.0f, set) == RUN_CLEANING);
    CHECK(nextRunState(RUN_CLEANING, true, 109.9f, set) == RUN_HEATING); // re-engage
    CHECK(nextRunState(RUN_CLEANING, false, 0.0f, set) == RUN_NO_SENSOR);
    CHECK(nextRunState(RUN_HEATING, false, 0.0f, set) == RUN_NO_SENSOR);
    CHECK(nextRunState(RUN_NO_SENSOR, true, 100.0f, set) == RUN_HEATING); // sensor back
    CHECK(nextRunState(RUN_NO_SENSOR, true, 111.0f, set) == RUN_CLEANING);
    CHECK(!runStateCounts(RUN_START) && !runStateCounts(RUN_HEATING));
    CHECK(runStateCounts(RUN_CLEANING) && runStateCounts(RUN_NO_SENSOR));

    // ---- a simulated run: 1 s steps, temperature profile; countdown only while cleaning ----
    {
        RunState s = RUN_START;
        uint32_t elapsed = 0;
        const uint32_t total = 60000; // 1 min
        float t = 100.0f;
        int heatingSteps = 0, reengaged = 0;
        RunState prev = s;
        for (int sec = 0; sec < 1000 && elapsed < total; sec++)
        {
            if (runStateCounts(s))
            {
                elapsed += 1000;
            }
            // heater warms 1 F/s; while cleaning the bowl cools 0.25 F/s
            t += (s == RUN_HEATING) ? 1.0f : -0.25f;
            s = nextRunState(s, true, t, set);
            if (s == RUN_HEATING)
            {
                heatingSteps++;
            }
            if (prev == RUN_CLEANING && s == RUN_HEATING)
            {
                reengaged++;
            }
            prev = s;
        }
        CHECK(elapsed == total);
        CHECK(heatingSteps > 0);
        CHECK(reengaged >= 1);
        printf("simulated run: %d heating seconds, heater re-engaged %d times\n", heatingSteps, reengaged);
    }

    // ---- countdown ----
    CHECK(remainingSeconds(600000, 0) == 600);
    CHECK(remainingSeconds(600000, 999) == 600);
    CHECK(remainingSeconds(600000, 1000) == 599);
    CHECK(remainingSeconds(600000, 599001) == 1);
    CHECK(remainingSeconds(600000, 600000) == 0);
    CHECK(remainingSeconds(600000, 700000) == 0);
    CHECK(remainingSeconds(3600000, 0) == 3600);

    // ---- units ----
    CHECK(displayTemp(118.4f, false) == 118);
    CHECK(displayTemp(118.5f, false) == 119);
    CHECK(displayTemp(212.0f, true) == 100);
    CHECK(displayTemp(32.0f, true) == 0);
    CHECK(displayTemp(-40.0f, true) == -40);
    CHECK(displayTemp(-196.6f, false) == -99);
    CHECK(displayTemp(5000.0f, false) == 999);
    CHECK(setTempFromC(16) == 61);  // 60.8 F
    CHECK(setTempFromC(0) == 61);   // clamped to 16 C
    CHECK(setTempFromC(82) == 180); // 179.6 F
    CHECK(setTempFromC(99) == 180);
    CHECK(setTempFromC(49) == 120); // 120.2 F
    CHECK(displayTemp(SET_TEMP_MIN_F, true) == SET_TEMP_MIN_C);
    CHECK(displayTemp(SET_TEMP_MAX_F, true) == SET_TEMP_MAX_C);
    // every C setting survives the F round trip
    for (int c = SET_TEMP_MIN_C; c <= SET_TEMP_MAX_C; c++)
    {
        const uint8_t f = setTempFromC(c);
        CHECK(f >= SET_TEMP_MIN_F && f <= SET_TEMP_MAX_F);
        CHECK(displayTemp(f, true) == c);
    }

    // ---- contrast mapping ----
    CHECK(contrastUiToRaw(20) == 80);
    CHECK(contrastUiToRaw(100) == 200);
    CHECK(contrastRawToUi(80) == 20);
    CHECK(contrastRawToUi(200) == 100);
    CHECK(contrastRawToUi(128) == 52);
    CHECK(contrastUiToRaw(52) == 128); // the default survives
    CHECK(contrastRawToUi(0) == 20 && contrastRawToUi(255) == 100);
    for (int ui = CONTRAST_UI_MIN; ui <= CONTRAST_UI_MAX; ui++)
    {
        const uint8_t raw = contrastUiToRaw(ui);
        CHECK(raw >= CONTRAST_MIN && raw <= CONTRAST_MAX);
        CHECK(contrastRawToUi(raw) == ui);
        if (ui > CONTRAST_UI_MIN)
        {
            CHECK(raw > contrastUiToRaw(ui - 1)); // every step changes the contrast
        }
    }

    // ---- encoder acceleration ----
    CHECK(encoderAccelSteps(ENC_GAP_NONE) == 1);
    CHECK(encoderAccelSteps(500) == 1);
    CHECK(encoderAccelSteps(90) == 1);
    CHECK(encoderAccelSteps(89) == 2);
    CHECK(encoderAccelSteps(40) == 2);
    CHECK(encoderAccelSteps(39) == 5);
    CHECK(encoderAccelSteps(5) == 5);
    {
        // contrast 20 -> 100 like adjustContrast(): one quick turn of a 20-detent encoder
        // (~0.4 s per turn, 20 ms between detents; the first detent has no gap)
        int ui = CONTRAST_UI_MIN;
        for (int d = 0; d < 20; d++)
        {
            const int steps = encoderAccelSteps(d == 0 ? ENC_GAP_NONE : 20);
            ui = (ui + steps < CONTRAST_UI_MAX) ? ui + steps : CONTRAST_UI_MAX;
        }
        CHECK(ui == CONTRAST_UI_MAX);
        // deliberate single clicks (200 ms apart) step by 1
        ui = 52;
        for (int d = 0; d < 3; d++)
        {
            ui += encoderAccelSteps(d == 0 ? ENC_GAP_NONE : 200);
        }
        CHECK(ui == 55);
    }

    // ---- progress ----
    CHECK(progressPx(0, 600000, 84) == 0);
    CHECK(progressPx(300000, 600000, 84) == 42);
    CHECK(progressPx(600000, 600000, 84) == 84);
    CHECK(progressPx(3600000, 3600000, 84) == 84);
    CHECK(progressPx(3599999, 3600000, 84) == 83);
    CHECK(progressPx(-5, 100, 84) == 0);
    CHECK(progressPx(5, 0, 84) == 84);
    CHECK(heatProgressPx(70.0f, 70.0f, 120.0f, 84) == 0);
    CHECK(heatProgressPx(70.0f, 91.0f, 120.0f, 84) == 42);
    CHECK(heatProgressPx(70.0f, 112.0f, 120.0f, 84) == 84);
    CHECK(heatProgressPx(70.0f, 60.0f, 120.0f, 84) == 0);  // cooled below the start
    CHECK(heatProgressPx(70.0f, 150.0f, 120.0f, 84) == 84); // overshoot

    // ---- big font sizes ----
    CHECK(bigTextWidth("00:23") == 76);
    CHECK(bigTextWidth("60:00") <= LCD_WIDTH);
    CHECK(bigTextWidth("DONE") == 69);
    CHECK(bigTextWidth("-99") == 51);
    CHECK(bigTextWidth("999") == 51);
    CHECK(BIG_TOP + BIG_CHAR_H <= 32);
    // every box of every big character stays inside its cell
    for (uint8_t i = 0; BIG_GLYPH_CHARS[i] != '\0'; i++)
    {
        const uint8_t cellW = (BIG_GLYPH_CHARS[i] == ':') ? BIG_COLON_W : BIG_CHAR_W;
        CHECK(BIG_GLYPH_FIRST[i] < BIG_GLYPH_FIRST[i + 1]);
        for (uint8_t k = BIG_GLYPH_FIRST[i]; k < BIG_GLYPH_FIRST[i + 1]; k++)
        {
            CHECK(BIG_BOXES[k].w > 0 && BIG_BOXES[k].h > 0);
            CHECK(BIG_BOXES[k].x + BIG_BOXES[k].w <= cellW);
            CHECK(BIG_BOXES[k].y + BIG_BOXES[k].h <= BIG_CHAR_H);
        }
    }

    printf("%s (%d failure%s)\n", failures ? "FAILED" : "all checks passed", failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
