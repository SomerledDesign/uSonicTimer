# uSonicTimer

Firmware for a timer and heater controller I added to a cheap ultrasonic cleaner. It runs on an
**ESP8266 (ESP-12E module)**. A **Nokia 5110 (PCD8544, 84x48) LCD** shows a menu that you drive with a
**rotary encoder and its push button**. A **DS18B20 1-Wire temperature sensor** measures the bowl
temperature. Two outputs drive **solid-state relays** that switch the 110 VAC bowl **heater** and the
ultrasonic **cleaner**. The set temperature, timer preset and LCD contrast are saved in (emulated)
EEPROM, so they survive a power cycle.

The menu approach comes from the [educ8s.tv](https://www.youtube.com/@Educ8s)
[menu tutorial](https://youtu.be/ak5TsUFhyf8?si=9JEMm8WbRyF4dVVC).

> Status: maintenance only.

## What it does

1. On power-up it loads the settings from EEPROM. If a value is unset it uses a default:
   72 °F, 10 min, contrast 64. It then initialises the LCD, the DS18B20, the encoder and the button.
2. It shows the main menu. You pick an item and a long press opens it.
3. **Start Timer** counts down the selected time and regulates the heater and cleaner:
   - While the bowl is more than `tempOffset` (10 °F) below the set temperature, the **heater is
     ON** and the **cleaner is OFF**.
   - Once the bowl is within 10 °F of the set temperature, the **heater is OFF** and the
     **cleaner is ON**.
   - When the countdown reaches 0, or you long-press the button, both outputs are switched off
     and the menu returns.

## Menu tree

In the menu, rotating the encoder moves the reverse-video highlight, a **long press** (> 1 s)
enters the highlighted item, and a **short press** toggles the LCD backlight. The backlight is on
at power-up. Its state is kept while you are in a page, because short presses inside the pages move
the cursor or confirm instead. There is no automatic backlight timeout. Every button press is handled
exactly once, so a press never carries over into the next page.

```
Main menu
├── Start Timer   "Time Left: m:ss" / "Temp: <current>F / <set>F"
│                 runs heater/cleaner as above; long press = abort (both outputs off)
├── Set Timer     "Set Timer: <n> min"
│                 rotate = choose preset 3, 8, 10, 15, 20, 30, 60 min (wraps)
│                 press = save to EEPROM and return
├── Set Temp      "Set Temp: ddd F" (3 digits, cursor digit in reverse video)
│                 short press = move cursor (hundreds → tens → units → …)
│                 rotate = change digit under cursor (0–9, wraps)
│                 long press = save to EEPROM and return
└── Contrast      "Contrast: ddd" (3 digits, cursor digit in reverse video)
                  short press = move cursor; rotate = ±100 / ±10 / ±1 (clamped 0–255, applied live)
                  long press = save to EEPROM and return
```

## Hardware (PCB rev 1d) pin map

| Signal | Arduino pin | ESP8266 GPIO | ESP-12E pin | Notes |
|---|---|---|---|---|
| LCD SCLK | D5 | GPIO14 | 5 | software SPI clock |
| LCD DIN | D7 | GPIO13 | 7 | software SPI data |
| LCD D/C | D10 | GPIO1 (TX) | 22 | shared with UART TX |
| LCD CS | D4 | GPIO2 | 17 | 10K pull-up |
| LCD RST | – | – | – | not connected (`U8X8_PIN_NONE`) |
| LCD backlight | D8 | GPIO15 | 16 | via Q1, 10K pull-down |
| Encoder CLK (A) | D0 | GPIO16 | 4 | 10K pull-up, polled (no interrupt on GPIO16) |
| Encoder DT (B) | D6 | GPIO12 | 6 | |
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

## Setup and usage

1. Wire the board as in the pin map, or use the rev 1d PCB. Put the DS18B20 on the bowl and
   connect the heater and cleaner through the SSRs.
2. Build and flash as above, then power up. The main menu appears.
3. Use **Set Temp** to choose the target bowl temperature (°F), **Set Timer** to choose the run
   time and **Contrast** to adjust the LCD if needed. Each setting is saved when you leave its page.
4. Highlight **Start Timer** and long-press. The display shows the remaining time and the current
   and set temperature while the heater and cleaner are controlled automatically.
5. Long-press at any time to stop. Both outputs switch off.

## Repository layout

- `src/main.cpp`: the firmware.
- `src/sketch.md`: design notes and the original specification.
- `include/`: headers (splash/footer glyph bitmaps).
- `handoff.md`: latest status and decisions.
- `extras/` (git-ignored): local reference material. This includes third-party menu/encoder demo
  code, links to the KiCad hardware projects and links to the datasheets, which are archived
  centrally.

## License

GNU GPL v3 or later. See the header of `src/main.cpp`.
