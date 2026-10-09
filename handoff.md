## Handoff

1. Latest request: 0.6.1 (73), a follow-up (acceleration, Press = save) to the 0.6.0 (72) UI
   milestone: big-digit run screens (heating / timer / DONE), countdown holds while heating with
   2 °F hysteresis, Settings... submenu, °F/°C units, backlight 0 = off, contrast shown as 20-100.
2. Current status: built (release and debug) and committed locally with tags `v0.6.0` and
   `v0.6.1`; not flashed or pushed yet. Screen previews: `tools/host/build.sh` (renders the real drawing code to PNGs).
3. Decisions:
   - Network/WiFi features are not needed for this project.
   - `release` is the default PlatformIO environment.
   - Heater on below set - 10 °F; cleaning and countdown resume at set - 8 °F. A run that starts
     within 10 °F of the set temperature starts cleaning straight away (as before).
   - Set time keeps the presets 3, 8, 10, 15, 20, 30, 60 min.
   - Backlight level 0 = off; the done pulse / fault blink then use level 5.
   - 0.6.1 (73): blocky big-digit font (box lists in include/bigfont.h) modelled on the
     cbm80amiga clock. "Press = save" in every settings page (Set temp: press = next digit, saves on
     the last digit; long press saves anywhere). Encoder acceleration only in Set contrast.
4. Open questions: flash and check the screens, the contrast mapping and the acceleration feel
   on the real hardware. Set time keeps its presets (decided).
