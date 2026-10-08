/**
 * @file ust_logic.h
 * @brief Run control, unit conversion and scaling helpers for uSonicTimer.
 *
 * Plain C++ without Arduino dependencies so the same code can be tested on a PC
 * (see tools/host/). All temperatures are degrees Fahrenheit unless the name says C.
 */
#pragma once

#include <stdint.h>
#include <math.h>

// ---- settings ranges -------------------------------------------------------
#define SET_TEMP_MIN_F    60
#define SET_TEMP_MAX_F    180
#define SET_TEMP_MIN_C    16  // 60 F = 15.6 C, rounded up so it stays inside the F range
#define SET_TEMP_MAX_C    82  // 180 F = 82.2 C, rounded down
#define CONTRAST_MIN      80  // raw U8g2 contrast; U8g2 sends contrast >> 1 as the PCD8544 Vop
#define CONTRAST_MAX      200 // outside 80..200 the display blanks
#define CONTRAST_DEFAULT  128 // = Vop 0x40, the U8g2 init value
#define CONTRAST_UI_MIN   20  // shown in the Set contrast page, mapped linearly onto 80..200
#define CONTRAST_UI_MAX   100

// ---- run control -------------------------------------------------------------
// The heater comes on below (set - 10 F). The cleaner and the countdown resume at (set - 8 F),
// so there is a 2 F gap between heating and cleaning. The heater is off while cleaning.
#define HEAT_ON_OFFSET_F  10.0f
#define RESUME_OFFSET_F   8.0f

enum RunState : uint8_t
{
    RUN_START,    // nothing decided yet (first pass)
    RUN_HEATING,  // heater on, cleaner off, countdown held
    RUN_CLEANING, // heater off, cleaner on, counting down
    RUN_NO_SENSOR // fail-safe: heater off, cleaner on, counting down without temperature control
};

/**
 * @brief Next run state from the current one and a temperature reading (hysteresis).
 * @param sensorOk false when the DS18B20 is missing/disconnected
 */
inline RunState nextRunState(RunState state, bool sensorOk, float tempF, float setF)
{
    if (!sensorOk)
    {
        return RUN_NO_SENSOR;
    }
    const float heatBelow = setF - HEAT_ON_OFFSET_F;
    const float resumeAt = setF - RESUME_OFFSET_F;
    switch (state)
    {
    case RUN_HEATING:
        return (tempF >= resumeAt) ? RUN_CLEANING : RUN_HEATING;
    case RUN_CLEANING:
        return (tempF < heatBelow) ? RUN_HEATING : RUN_CLEANING;
    default: // RUN_START, or the sensor just came back: decide like the old firmware did
        return (tempF < heatBelow) ? RUN_HEATING : RUN_CLEANING;
    }
}

/// true when the countdown (and the elapsed run time) advances in this state
inline bool runStateCounts(RunState state)
{
    return state == RUN_CLEANING || state == RUN_NO_SENSOR;
}

/// whole seconds still to run, rounded up (shows 10:00 for the first second of a 10 min run)
inline uint16_t remainingSeconds(uint32_t totalMs, uint32_t elapsedMs)
{
    if (elapsedMs >= totalMs)
    {
        return 0;
    }
    return static_cast<uint16_t>((totalMs - elapsedMs + 999UL) / 1000UL);
}

// ---- rounding and units ------------------------------------------------------
inline int16_t roundToInt(float v)
{
    if (v > 32000.0f)
    {
        return 32000;
    }
    if (v < -32000.0f)
    {
        return -32000;
    }
    return static_cast<int16_t>(floorf(v + 0.5f));
}

inline float fToC(float f) { return (f - 32.0f) * 5.0f / 9.0f; }
inline float cToF(float c) { return c * 9.0f / 5.0f + 32.0f; }

/// whole degrees in the display unit, clamped to -99..999 so it always fits the big font
inline int16_t displayTemp(float tempF, bool celsius)
{
    const int16_t v = roundToInt(celsius ? fToC(tempF) : tempF);
    return (v < -99) ? -99 : (v > 999) ? 999 : v;
}

/// a set temperature entered in C, stored as whole F within SET_TEMP_MIN_F..SET_TEMP_MAX_F
inline uint8_t setTempFromC(int16_t c)
{
    if (c < SET_TEMP_MIN_C)
    {
        c = SET_TEMP_MIN_C;
    }
    if (c > SET_TEMP_MAX_C)
    {
        c = SET_TEMP_MAX_C;
    }
    int16_t f = roundToInt(cToF(c));
    f = (f < SET_TEMP_MIN_F) ? SET_TEMP_MIN_F : (f > SET_TEMP_MAX_F) ? SET_TEMP_MAX_F : f;
    return static_cast<uint8_t>(f);
}

// ---- contrast ----------------------------------------------------------------
/// raw 80..200 -> shown 20..100 (rounded)
inline uint8_t contrastRawToUi(uint8_t raw)
{
    if (raw < CONTRAST_MIN)
    {
        raw = CONTRAST_MIN;
    }
    if (raw > CONTRAST_MAX)
    {
        raw = CONTRAST_MAX;
    }
    const uint16_t uiSpan = CONTRAST_UI_MAX - CONTRAST_UI_MIN;
    const uint16_t rawSpan = CONTRAST_MAX - CONTRAST_MIN;
    return static_cast<uint8_t>(CONTRAST_UI_MIN + ((raw - CONTRAST_MIN) * uiSpan + rawSpan / 2) / rawSpan);
}

/// shown 20..100 -> raw 80..200 (rounded)
inline uint8_t contrastUiToRaw(uint8_t ui)
{
    if (ui < CONTRAST_UI_MIN)
    {
        ui = CONTRAST_UI_MIN;
    }
    if (ui > CONTRAST_UI_MAX)
    {
        ui = CONTRAST_UI_MAX;
    }
    const uint16_t uiSpan = CONTRAST_UI_MAX - CONTRAST_UI_MIN;
    const uint16_t rawSpan = CONTRAST_MAX - CONTRAST_MIN;
    return static_cast<uint8_t>(CONTRAST_MIN + ((ui - CONTRAST_UI_MIN) * rawSpan + uiSpan / 2) / uiSpan);
}

// ---- progress bars -------------------------------------------------------------
/// num/den of width pixels, clamped to 0..width (den <= 0 counts as complete)
inline uint8_t progressPx(int32_t num, int32_t den, uint8_t width)
{
    if (den <= 0 || num >= den)
    {
        return width;
    }
    if (num <= 0)
    {
        return 0;
    }
    return static_cast<uint8_t>((static_cast<int64_t>(num) * width) / den);
}

/// heating progress from the temperature the heater came on at to the resume threshold (set - 8 F)
inline uint8_t heatProgressPx(float startF, float nowF, float setF, uint8_t width)
{
    const float target = setF - RESUME_OFFSET_F;
    const float span = target - startF;
    if (span < 0.1f)
    {
        return (nowF >= target) ? width : 0;
    }
    float frac = (nowF - startF) / span;
    if (!(frac > 0.0f)) // also catches NaN
    {
        return 0;
    }
    if (frac >= 1.0f)
    {
        return width;
    }
    return static_cast<uint8_t>(frac * width);
}
