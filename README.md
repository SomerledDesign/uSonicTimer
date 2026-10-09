# uSonicTimer

Firmware for a timer and heater controller I added to a cheap ultrasonic cleaner. It runs on an
**ESP8266 (ESP-12E module)**. A **Nokia 5110 (PCD8544, 84x48) LCD** shows a menu that you drive with a
**rotary encoder and its push button**. A **DS18B20 1-Wire temperature sensor** measures the bowl
temperature. Two outputs drive **solid-state relays** that switch the 110 VAC bowl **heater** and the
ultrasonic **cleaner**. The set temperature, timer preset, display unit (°F/°C), LCD contrast and
backlight brightness are saved in (emulated) EEPROM, so they survive a power cycle.

The menu approach comes from the [educ8s.tv](https://www.youtube.com/@Educ8s)
[menu tutorial](https://youtu.be/ak5TsUFhyf8?si=9JEMm8WbRyF4dVVC).

> Status: maintenance only.

## What it does

1. On power-up it loads the settings from EEPROM. If a value is unset or out of range it uses a
   default: 72 °F, 10 min, °F, contrast 52 (raw 128), backlight level 10. It then initialises the
   LCD, the DS18B20, the encoder and the button.
2. It shows the main menu. You pick an item and a long press opens it.
3. **Start** runs the cleaner for the set time and regulates the heater (the control logic always
   works in °F, with tenths; the screens show whole degrees in the chosen unit):
   - **Heating:** below set − 10 °F the **heater is ON**, the **cleaner is OFF** and the countdown
     **holds**. The screen shows the bowl temperature in big digits and "Heating to 120°F"; the
     rule under the digits fills as the bowl warms from where heating started to set − 8 °F.
   - **Cleaning:** at set − 8 °F or above (2 °F hysteresis) the **heater is OFF**, the **cleaner is
     ON** and the countdown runs. The screen shows the time left as big `MM:SS` with a blinking
     colon, the rule under it fills with the run's progress, and "Now 118 Set 120°F" underneath.
     If the bowl drops below set − 10 °F the heater comes back on and the countdown holds again.
     Only cleaning time counts towards the run.
   - **No sensor:** heater locked off, cleaner on, the countdown runs; the time screen shows
     "NO SENSOR" instead of the temperatures.
   - When the time is up both outputs switch off and a big **DONE** stays on screen until you turn
     or press; then the menu returns. A long press during the run aborts it (both outputs off).

## Menu tree

In the menus, rotating the encoder moves the reverse-video highlight (lists show 4 rows under the
title bar and scroll; small arrows mark more items), a **long press** (> 1 s) enters the
highlighted item, and a **short press** toggles the LCD backlight between off and the level set in
**Set backlight**. The backlight is on at power-up.
Its state is kept while you are in a page, because short presses inside the pages move the cursor
or confirm instead. After 5 minutes without input in the menu the backlight dims to about 20 %; the
first turn or press then only wakes it. Every button press is handled exactly once, so a press never
carries over into the next page.

**Encoder test:** hold the button while powering up (or pressing reset) to get a test screen with the
live A/B levels, the detent count and how often each input has changed; long press returns to the
menu. Both edge counts should rise together while turning. `ENCODER_STEPS_PER_DETENT` in
`src/main.cpp` is 2 for an encoder whose detents rest at both 00 and 11, 4 for one that always rests
at the same state.

The backlight also shows the status (it swings between the set brightness and about 30 % of it, so
the screen stays readable):

- **Steady**: normal operation (menu, cleaning).
- **Flashing** (1 Hz): heating up.
- **Pulsing** (slow): the timer has finished; stops at the next turn or press.
- **Double-blink** every 2 s: no temperature sensor during a run (heater locked off, cleaner runs).

Pulsing and double-blink also show when the backlight has been toggled off or set to level 0 (then
at level 5); heating and cleaning then stay dark.

```
Main menu "uSonicTimer"      (shows the set temperature and time underneath)
├── Start             heating screen (big temperature) / timer screen (big MM:SS) / DONE, as above
│                     long press = abort (both outputs off)
└── Settings...
    ├── Set temp      "ddd°F" (3 digits) or "dd°C" (2 digits), cursor digit in reverse video
    │                 rotate = change digit (0–9, wraps); press = next digit ("Press = next"),
    │                 press on the last digit = save and return ("Press = save");
    │                 long press = save from any digit (clamped 60–180 °F / 16–82 °C, stored in °F)
    ├── Set time      "<n> min"; rotate = preset 3, 8, 10, 15, 20, 30, 60 min (wraps)
    │                 press = save and return
    ├── Set units     "°F Fahrenheit" / "°C Celsius"; rotate = toggle, press = save and return
    ├── Set backlight "Off" / "Level 1"…"Level 10" with a bar; rotate = level (previewed live)
    │                 press = save, switch the backlight on and return
    ├── Set contrast  20–100 with a bar (mapped onto the usable raw 80–200); clockwise = higher,
    │                 accelerated (detents < 40 ms apart = 5 steps, < 90 ms = 2, else 1),
    │                 applied live; press = save and return
    └── Exit          back to the main menu
```

Settings are saved to EEPROM when you leave their page: set temperature `0x00` (°F), time `0x08`,
contrast `0x10` (raw 80–200, so the 0.5.0 value carries over), backlight level `0x18`, units `0x20`
(0 = °F, 1 = °C) and a layout id `0x28` (0x60) that marks EEPROM written by 0.6.0 or later. Every
value is range-checked on load; a backlight level of 0 is only accepted together with that id.

## Hardware (PCB rev 1d) pin map

The KiCad design files for this board are in [`hardware/uSonicTimer_1d/`](hardware/uSonicTimer_1d/).

| Signal | Arduino pin | ESP8266 GPIO | ESP-12E pin | Notes |
|---|---|---|---|---|
| LCD SCLK | D5 | GPIO14 | 5 | software SPI clock |
| LCD DIN | D7 | GPIO13 | 7 | software SPI data |
| LCD D/C | D10 | GPIO1 (TX) | 22 | shared with UART TX |
| LCD CS | D4 | GPIO2 | 17 | 10K pull-up |
| LCD RST | – | – | – | not connected (`U8X8_PIN_NONE`) |
| LCD backlight | D8 | GPIO15 | 16 | via Q1, 10K pull-down |
| Encoder CLK (A) | D0 | GPIO16 | 4 | 4K7 pull-up (R9), polled (no interrupt on GPIO16) |
| Encoder DT (B) | D6 | GPIO12 | 6 | 4K7 pull-up (R10), polled |
| Encoder SW | D9 | GPIO3 (RX) | 21 | shared with UART RX |
| Heater SSR | D2 | GPIO4 | 19 | HIGH = ON |
| Cleaner SSR | D1 | GPIO5 | 20 | HIGH = ON |
| DS18B20 1-Wire | D3 | GPIO0 | 18 | 4K7 pull-up; also ~PROGRAM strap |

On rev 1d, NPN transistors switch the heater and cleaner SSRs low-side. The logic is the same as
before: GPIO HIGH = ON.

The programming header is a set of 5 pogo pads: 1 GND, 2 RTS, 3 TX, 4 RX, 5 DTR. RTS and DTR
drive the auto-reset/boot circuit (Q2/Q3).

Because the UART pins are also used for the LCD D/C line and the encoder switch, serial output
from the `debug` build shares those lines with the hardware.

## Building and flashing (PlatformIO)

The project uses the `espressif8266` platform, board `esp12e` and the Arduino framework. It needs
these libraries, which PlatformIO fetches automatically from `platformio.ini`:
`olikraus/U8g2`, `lennarthennigs/ESP Rotary`, `lennarthennigs/Button2`, `paulstoffregen/OneWire`
and `milesburton/DallasTemperature`. `EEPROM` and `Ticker` come with the ESP8266 core.

```sh
pio run -e release                  # build the release firmware
pio run -e release -t upload        # flash it over the programming header
pio device monitor -b 115200        # serial monitor (debug output)
```

- `release` is the default environment (`default_envs = release`); it builds without the debug
  output.
- `debug` (`pio run -e debug`) adds `-Og -ggdb -g3 -D DEBUG -D WITH_GDB`. It enables the
  serial `debug()`/`debugln()` output at 115200 baud.
- `platformio.ini` puts `build_dir` and `libdeps_dir` under
  `$HOME/Library/Caches/PlatformIO/uSonicTimer/`, which keeps build output out of Dropbox. On a
  non-macOS machine, change these paths or remove them to use the default `.pio/` folder.

## Versioning

Firmware versions are a semantic version plus a build number, shown as **`0.6.1 (73)`** (current).

- Bump PATCH for fixes and MINOR for features; 1.0.0 is the first version installed and in service.
- The build number goes up by 1 for every build flashed for testing and never resets.
- `FW_VERSION`, `FW_BUILD` and `HW_REV` at the top of `src/main.cpp` set it. The startup screen
  shows `uSonicTimer` / `v0.6.1 (73)` / `PCB Rev D` for 1.5 s at power-up. Releases are tagged
  `vMAJOR.MINOR.PATCH` in git.

History:

- **0.6.1 (73)**: new big-digit font: blocky, squared-off 7-segment-style characters (4–5 px
  strokes, no diagonals, stepped slashed zero, colon of two 4x4 blocks) drawn from box lists in
  `include/bigfont.h`, closer to the reference clock than the scaled 5x7 font. Encoder
  acceleration in Set contrast (5 / 2 / 1 steps per detent for detents
  < 40 ms / < 90 ms / slower apart). A press saves in every settings page ("Press = save"); in Set
  temp a press moves to the next digit and saves on the last one, long press still saves anywhere.
  Detents still queued when a page closes no longer move the menu highlight.
- 0.6.0 (72): big-digit run screens in the style of a Nokia 5110 clock (Adafruit 5x7 digits
  scaled to 15x28 px, a rule that doubles as a progress bar, one line of small text): heating
  screen with the bowl temperature, timer screen with blinking colon, DONE screen. The countdown
  holds while heating (heater below set − 10 °F, cleaning resumes at set − 8 °F). Main menu is now
  Start / Settings...; Settings has Set temp, Set time, Set units (°F/°C, new), Set backlight
  (0 = off, new), Set contrast (shown as 20–100) and Exit, with title bars.
- 0.5.0 (71), `2d4bacc`: version defines, startup screen, versioning notes.
- Build 70, `a255348`: polled quadrature decoder (replaces ESPRotary) and the encoder test screen.
- Build 69, `44c0249`: backlight as status indicator and the Backlight brightness menu (`90cbc71`).

## Setup and usage

1. Wire the board as in the pin map, or use the rev 1d PCB. Put the DS18B20 on the bowl and
   connect the heater and cleaner through the SSRs.
2. Build and flash as above, then power up. The main menu appears.
3. Open **Settings...** and use **Set temp** to choose the target bowl temperature, **Set time**
   to choose the run time, **Set units** for °F or °C, and **Set backlight** / **Set contrast** for
   the LCD if needed. Each setting is saved when you leave its page.
4. Highlight **Start** and long-press. The display shows the bowl temperature while it heats, then
   the time left while it cleans; the heater and cleaner are controlled automatically.
5. Long-press at any time to stop. Both outputs switch off. When the time is up, DONE shows until
   you turn or press.

## Repository layout

- `src/main.cpp`: the firmware (setup, menus, pages, run control, EEPROM, backlight).
- `src/screens.cpp`, `include/screens.h`: drawing for the run screens, menus and settings pages.
- `include/ust_logic.h`: run-control hysteresis, °F/°C, contrast and progress helpers (no Arduino
  dependencies).
- `include/bigfont.h`: the big-digit font (blocky 15x28 characters as box lists, slashed zero).
- `tools/host/`: host-side test of `ust_logic.h` and a screen preview that runs the real drawing
  code against the U8g2 C library on a PC (`tools/host/build.sh`, writes PNGs to
  `tools/host/out/`).
- `src/sketch.md`: design notes and the original specification.
- `include/`: also the old splash/footer glyph bitmaps.
- `handoff.md`: latest status and decisions.
- `hardware/uSonicTimer_1d/`: KiCad project for the rev 1d controller board (schematic, PCB,
  project symbol/footprint libraries, Gerbers, interactive BOM and renders).
- `extras/` (git-ignored): local reference material. This includes third-party menu/encoder demo
  code, links to the KiCad hardware projects and links to the datasheets, which are archived
  centrally.

## License

uSonicTimer uses two licences, one for the code and one for the hardware:

- **Firmware and code** (`src/`, `include/`, `platformio.ini` and the rest of the software) are
  under the GNU General Public License, version 3 or later. See [`LICENSE`](LICENSE).
  SPDX-License-Identifier: GPL-3.0-or-later
- **Hardware design files** (everything under [`hardware/`](hardware/): the KiCad schematic,
  PCB, libraries and related fabrication files for the controller board, and the same files
  wherever else they are published for this project) are under the CERN Open Hardware Licence
  Version 2, Strongly Reciprocal. See [`LICENSE-HARDWARE`](LICENSE-HARDWARE).
  SPDX-License-Identifier: CERN-OHL-S-2.0

Both licences come with no warranty. This controller switches 110 VAC mains through solid-state
relays; build and use it at your own risk.

Copyright © 2024 Somerled Design, LLC.
