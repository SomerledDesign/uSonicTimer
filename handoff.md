## Handoff

1. Latest request: 0.6.0 (72) UI milestone: big-digit run screens (heating / timer / DONE), countdown
   holds while heating with 2 °F hysteresis, Settings... submenu, °F/°C units, backlight 0 = off,
   contrast shown as 20-100.
2. Current status: built (release and debug) and committed locally with tag `v0.6.0`; not flashed or
   pushed yet. Screen previews: `tools/host/build.sh` (renders the real drawing code to PNGs).
3. Decisions:
   - Network/WiFi features are not needed for this project.
   - `release` is the default PlatformIO environment.
   - Heater on below set - 10 °F; cleaning and countdown resume at set - 8 °F. A run that starts
     within 10 °F of the set temperature starts cleaning straight away (as before).
   - Set time keeps the presets 3, 8, 10, 15, 20, 30, 60 min.
   - Backlight level 0 = off; the done pulse / fault blink then use level 5.
4. Open questions: flash and check the screens and the contrast mapping on the real LCD; should
   Set time allow any 1-60 min instead of the presets?
