# Changelog

What changed in each release. New notes go under **Unreleased** while the work is done; a release moves them under its version, and the release workflow publishes that section as the GitHub release notes (it refuses to release a version without them).

## Unreleased

- **Fix:** the dashboard could open without the holiday settings, and with the default power limit instead of yours: the device sent garbage in place of the greetings, which broke its answer. The greetings are sent as they are saved again, and when an answer can't be read the dashboard now says "Couldn't read the device" instead of showing the defaults. Flash the new firmware; your settings and greetings are kept.
- gizmo-common v2.7.0: when the device sends an answer the web app can't read, it now shows an error ("The device sent an answer the app could not read.") instead of carrying on with empty values. Flash the new firmware to get it.

## v1.3.0 - 2026-11-20

Easter: a new holiday, with eight scenes between spring and the Resurrection.

- **Easter**, a new holiday with eight scenes. On **Automatic** it plays from Palm Sunday to Easter Monday (March 21 to 29 in 2027), and you can pick it any day.
  - Spring: **Greeting** among blooming flowers, **Egg hunt** (a bunny hiding eggs in the meadow), **Painted eggs** and **Hatching chick**.
  - Religious: **Three crosses** on a hill at sunrise, **Empty tomb** (the stone rolls away, then `CRISTO E' RISORTO!` or `HE IS RISEN!`), **Dove of peace** with an olive branch, and **Easter bells**.
  - Its own greeting, `BUONA PASQUA!` or `HAPPY EASTER!` following the language until you change it.
  - Flash the new firmware; your settings and other greetings are kept.

## v1.2.0 - 2026-11-18

Holidays: the strip now plays Christmas or New Year's Eve, picked by the calendar or by you, and runs on an ESP32 DevKit.

- **New board: ESP32 DevKit (ESP-WROOM-32)** instead of the XIAO ESP32-S3. Move the matrix data wire to **GPIO16**. Flash it over USB, and set up the Wi-Fi again from the `Holiday-Strip` hotspot. This release doesn't run on the XIAO.
- **Holidays.** **Automatic** follows the calendar: New Year's Eve from December 27 to January 1, Christmas the rest of the year. The strip fades to the next holiday when the date changes. You can also keep one holiday, or one scene of it, all the time.
- **New Year's Eve**, with five scenes:
  - **Countdown**: the time left to midnight on December 31, the last ten seconds one by one, then the new year bursting in. In the last minute of the year the strip switches to it by itself.
  - **Confetti**: your New Year's greeting under falling confetti.
  - **Fireworks**: moved here from Christmas.
  - **Champagne**: a bottle pops, and two glasses fill and clink.
  - **Sparklers**: a sparkler writes the coming year in the air.
- **Countdown to Christmas** is now a Christmas scene of its own, all year: the days left between two little trees, then the sentence scrolling. It replaces the countdown switch and the countdown after the greeting.
- **Custom message**, picked like a holiday: just your text scrolling, any day of the year, in white, warm white, red, green, blue, gold or rainbow.
- **A message per holiday.** Christmas, New Year's Eve and the custom message each have their own text.
- **The texts on the matrix follow the dashboard's language.** That's the Christmas countdown, and the greetings you haven't changed (`BUON NATALE!` or `MERRY CHRISTMAS!`). Pick the language in the top bar.
- **The dashboard**, with the same tabs as every OpenGizmo app:
  - **Strip** has the preview, the display, the holiday and, under it, its scene and custom message, then the power limit.
  - **Device** is the device card.
  - **WiFi** is unchanged.
  - The Message and Settings tabs are gone.
- gizmo-common v2.6.0: the Device tab shows the address that opens the strip (`holiday-strip.local`), and the Wi-Fi reconnects more reliably.

## v1.1.0 - 2026-11-08

- **Factory reset**: hold the BOOT button for 10 seconds to forget the Wi-Fi and the settings, and start again from the setup hotspot.
- The dashboard only answers its own page, so other websites can't change the settings from your browser (gizmo-common v2.4.0).

## v1.0.3 - 2026-11-08

- The `huge_app` partition table, like every OpenGizmo firmware: one 3 MB app slot. Flash it once over USB; the settings and the Wi-Fi are kept.

## v1.0.2 - 2026-11-08

- The firmware updates over USB only (no more wireless updates).
- The `release` and `debug` build profiles of every OpenGizmo firmware, and gizmo-common v2.3.2.

## v1.0.1 - 2026-11-08

- No stray open `ESP_XXXXXX` hotspot: the setup hotspot only starts when no saved network works, with its own name, and really switches off (gizmo-common v2.3.1).

## v1.0.0 - 2026-11-08

The first release of Holiday Strip: nine animated Christmas scenes on a 72x12 truck LED matrix, on a Seeed Studio XIAO ESP32-S3.

- Scenes that rotate with crossfades, your greeting scrolling, and a countdown to Christmas.
- A dashboard built into the firmware, in English and Italian, with a live preview, the scene picker, the message and the power limit.
- Wi-Fi setup from the phone, through the `Holiday-Strip` hotspot, then `holiday-strip.local` on your network.