# uSonicTimer

Firmware for a timer and heater controller I added to a cheap ultrasonic cleaner. It runs on an
**ESP8266 (ESP-12E module)**. A **Nokia 5110 (PCD8544, 84x48) LCD** shows a menu that you drive with a
**rotary encoder and its push button**. A **DS18B20 1-Wire temperature sensor** measures the bowl
temperature. Two outputs drive **solid-state relays** that switch the 110 VAC bowl **heater** and the
ultrasonic **cleaner**. The set temperature, timer preset, LCD contrast and backlight brightness are
saved in (emulated) EEPROM, so they survive a power cycle.

The menu approach comes from the [educ8s.tv](https://www.youtube.com/@Educ8s)
[menu tutorial](https://youtu.be/ak5TsUFhyf8?si=9JEMm8WbRyF4dVVC).

> Status: maintenance only.

## What it does

1. On power-up it loads the settings from EEPROM. If a value is unset or out of range it uses a
   default: 72 °F, 10 min, contrast 128, backlight 100 %. It then initialises the LCD, the DS18B20,
   the encoder and the button.
2. It shows the main menu. You pick an item and a long press opens it.
3. **Start Timer** counts down the selected time and regulates the heater and cleaner:
   - While the bowl is more than `tempOffset` (10 °F) below the set temperature, the **heater is
     ON** and the **cleaner is OFF**.
   - Once the bowl is within 10 °F of the set temperature, the **heater is OFF** and the
     **cleaner is ON**.
   - When the countdown reaches 0, or you long-press the button, both outputs are switched off
     and the menu returns.

## Menu tree

In the menu, rotating the encoder moves the reverse-video highlight (the menu shows 4 rows and
scrolls), a **long press** (> 1 s) enters the highlighted item, and a **short press** toggles the
LCD backlight between off and the brightness set in **Backlight**. The backlight is on at power-up.
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

Pulsing and double-blink also show when the backlight has been toggled off; heating and cleaning
then stay dark.

```
Main menu
├── Start Timer   "Time Left:" / "mm:ss" / "Now: <current>F" (or "Temp: --") / "Set: <set>F"
│                 runs heater/cleaner as above; long press = abort (both outputs off)
├── Set Timer     "Set Timer:" / "<n> min"
│                 rotate = choose preset 3, 8, 10, 15, 20, 30, 60 min (wraps)
│                 press = save to EEPROM and return
├── Set Temp      "Set Temp:" / "ddd F" (3 digits, cursor digit in reverse video)
│                 short press = move cursor (hundreds → tens → units → …)
│                 rotate = change digit under cursor (0–9, wraps)
│                 long press = save to EEPROM and return
├── Contrast      "Contrast:" / ddd (3 digits, cursor digit in reverse video)
│                 short press = move cursor; rotate clockwise = +100 / +10 / +1
│                 (clamped 80–200, applied live); long press = save to EEPROM and return
└── Backlight     "Backlight:" / "[#######   ]" / "70%"
                  rotate = brightness 10–100 % in 10 % steps (clockwise = brighter, previewed live)
                  long press = save to EEPROM, switch the backlight on and return
```

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

- `debug` is the default environment: `-Og -ggdb -g3 -D DEBUG -D WITH_GDB`. It enables the
  serial `debug()`/`debugln()` output at 115200 baud.
- `release` builds without the debug output.
- `platformio.ini` puts `build_dir` and `libdeps_dir` under
  `$HOME/Library/Caches/PlatformIO/uSonicTimer/`, which keeps build output out of Dropbox. On a
  non-macOS machine, change these paths or remove them to use the default `.pio/` folder.

## Versioning

Firmware versions are a semantic version plus a build number, shown as **`0.5.0 (71)`** (current).

- Bump PATCH for fixes and MINOR for features; 1.0.0 is the first version installed and in service.
- The build number goes up by 1 for every build flashed for testing and never resets.
- `FW_VERSION`, `FW_BUILD` and `HW_REV` at the top of `src/main.cpp` set it. The startup screen
  shows `uSonicTimer` / `v0.5.0 (71)` / `PCB Rev D` for 1.5 s at power-up. Releases are tagged
  `vMAJOR.MINOR.PATCH` in git.

Earlier builds: build 69 = `44c0249`, build 70 = `a255348` (backlight status/Backlight menu and the
polled encoder decoder).

## Setup and usage

1. Wire the board as in the pin map, or use the rev 1d PCB. Put the DS18B20 on the bowl and
   connect the heater and cleaner through the SSRs.
2. Build and flash as above, then power up. The main menu appears.
3. Use **Set Temp** to choose the target bowl temperature (°F), **Set Timer** to choose the run
   time, **Contrast** to adjust the LCD and **Backlight** to set its brightness if needed. Each
   setting is saved when you leave its page.
4. Highlight **Start Timer** and long-press. The display shows the remaining time and the current
   and set temperature while the heater and cleaner are controlled automatically.
5. Long-press at any time to stop. Both outputs switch off.

## Repository layout

- `src/main.cpp`: the firmware.
- `src/sketch.md`: design notes and the original specification.
- `include/`: headers (splash/footer glyph bitmaps).
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
