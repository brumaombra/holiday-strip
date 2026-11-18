<div align="center">

# 🎄 Holiday Strip

### An ESP32-powered holiday show for a 72x12 LED matrix

Holiday Strip is a small ESP32 project that turns a 72x12 truck LED matrix into an animated holiday display. It has a set of hand-drawn scenes for each holiday, Christmas and New Year's Eve so far, that rotate with smooth crossfades, and your own greeting scrolling, with a countdown to Christmas and to midnight on New Year's Eve. It follows the calendar by itself, and a phone-friendly dashboard, built into the firmware, controls it all.

<p>
  <img alt="ESP32" src="https://img.shields.io/badge/esp32-ESP32-E7352C?logo=espressif&logoColor=white">
  <img alt="PlatformIO" src="https://img.shields.io/badge/platformio-PlatformIO-FF6B35?logo=platformio&logoColor=white">
  <img alt="Arduino" src="https://img.shields.io/badge/framework-Arduino-00979D?logo=arduino&logoColor=white">
  <img alt="Nuxt 4" src="https://img.shields.io/badge/nuxt-4-00DC82?logo=nuxt&logoColor=white">
  <img alt="WS2812B" src="https://img.shields.io/badge/leds-864%20WS2812B-16A34A">
  <img alt="Matrix 72x12" src="https://img.shields.io/badge/display-72x12%20matrix-0F766E">
  <img alt="License MIT" src="https://img.shields.io/badge/license-MIT-2563EB">
</p>

<p>
  🎅 Christmas and 🎆 New Year's Eve • 💬 Scrolling greetings • ⏳ Countdowns • 📱 Web dashboard • 🔌 Power limiter
</p>

<p>
	<a href="#overview"><strong>Overview</strong></a> •
	<a href="#features"><strong>Features</strong></a> •
	<a href="#architecture"><strong>Architecture</strong></a> •
	<a href="#quick-start"><strong>Quick Start</strong></a> •
	<a href="#hardware"><strong>Hardware</strong></a> •
	<a href="#firmware-overview"><strong>Firmware Overview</strong></a> •
	<a href="#configuration"><strong>Configuration</strong></a> •
	<a href="#troubleshooting"><strong>Troubleshooting</strong></a> •
	<a href="#license"><strong>License</strong></a>
</p>

</div>

<a id="overview"></a>
## 📘 Overview

This project started from a 72x12 LED matrix pulled from a truck sign. Its protocol was worked out by driving it from an ESP32 and watching it with an ESP32-CAM (see [docs/protocol.md](docs/protocol.md)), and now it shows holiday scenes. Christmas and New Year's Eve are the first holidays, and more will follow.

The firmware runs on an ESP32 and:

- Plays the scenes of a holiday: the one the calendar picks (New Year's Eve from December 27 to January 1, Christmas the rest of the year), or the one you choose. Or, any day, just your custom message, in the color you choose.
- Rotates through the scenes every 20 seconds, with a 1-second crossfade, and fades to the next holiday when the date changes.
- Scrolls a message per holiday, counts the days to Christmas from late October, and counts down to midnight on December 31 (in the last minute of the year it switches to the countdown by itself).
- Serves a dashboard on your network to switch it on and off, set the brightness, watch a live preview, pick a holiday and a scene, and edit the messages.
- Estimates the current of every frame and dims it before it goes over your power supply's limit.

### 🎄 Christmas scenes

1. **Greeting**: your message scrolls between two chasing garlands. In the 60 days before Christmas it adds a countdown (`MANCANO N GIORNI A NATALE!` or `N DAYS TO CHRISTMAS!`), and on December 25 it shows `OGGI E' NATALE!` or `IT'S CHRISTMAS DAY!`, in the language picked on the dashboard.
2. **Snowfall**: snow falls and piles up next to a little house with warm windows.
3. **Christmas trees**: six trees with twinkling ornaments and gold stars, in falling snow.
4. **Santa's sleigh**: Santa and two reindeer fly across a starry sky with a moon. Rudolph's nose blinks red, and the sleigh leaves a trail of sparkles.
5. **Fireplace**: a brick fireplace with a simulated fire and glowing logs. Red and green stockings hang from the mantel.
6. **Fairy lights**: two strings of lights hanging in swags. They alternate between twinkle, chase, and color-cycle effects.
7. **Gifts**: wrapped presents with ribbons and bows fall and stack up. When the pile gets too high, it sinks away and starts over.
8. **Candy cane**: shaded red and white stripes sliding along, with sparkles.

### 🎆 New Year's Eve scenes

1. **Countdown**: on December 31, the time left to midnight (hours, minutes and seconds, then minutes and seconds, then seconds), the last ten seconds one by one in a flashing ring, then the new year bursting in with a white flash and sparks for the first two minutes. The other days it shows the coming year with a glint sweeping across it.
2. **Confetti**: the New Year's greeting scrolls in gold and silver under falling, spinning confetti.
3. **Fireworks**: rockets launch and burst into colored sparks over a dim gold year number. From November on, it shows the coming year.
4. **Champagne**: a bottle shakes and pops its cork, foam sprays out, then two glasses fill with bubbles, slide together and clink.
5. **Sparklers**: a sparkler writes the coming year in the air. The trail cools from white-hot to an orange glow while sparks spit from the tip, then it fades and starts over.

### 💬 Custom message

Picked like a holiday (the calendar never picks it), for any day of the year: your own message scrolling on black, in the color you choose on the dashboard (white, warm white, red, green, blue, gold, or a rainbow drifting through the letters).

Every scene can be watched on your computer, without a board: `microcontroller/tools/scene_video/render.sh` builds the real scene code for your computer and records it to an MP4 (see [Scene videos](#scene-videos)).

<a id="features"></a>
## ✨ Features

- Thirteen animated scenes in two holidays, at about 30 fps, rotating with crossfades, or one scene shown on its own.
- The holiday follows the calendar, or stays on the one you pick.
- A scrolling greeting per holiday, in capitals (letters A–Z, digits, and `! ' * , - . : ?`, while `<` draws a heart).
- Christmas countdown and New Year's midnight countdown that follow the local timezone, daylight saving time included.
- Web dashboard built into the firmware, with a live preview of the matrix, the holiday picker with the settings of each holiday under it, and the power limit.
- Wi-Fi setup from your phone: with no saved network, the matrix opens a setup hotspot with a captive portal.
- Settings saved in flash, so they survive a reboot.
- Gamma-corrected brightness and a per-frame power limiter, capped below what the power supply can deliver.
- A JSON API that only answers the dashboard's own page, under the matrix's own name or address, so other websites can't change the settings from your browser. Through the setup hotspot, only the Wi-Fi setup answers.
- Factory reset: hold the board's BOOT button for 10 seconds while it runs to forget the Wi-Fi and the settings, and start again from the setup hotspot.
- Host tests for the LED mapping, the power limiter, the countdowns, the holiday calendar and the settings, and Vitest tests for the web app.

<a id="architecture"></a>
## 🏗️ Architecture

Holiday Strip is built around three cooperating parts:

- 🔌 **ESP32 firmware** draws the scenes into a frame buffer, mixes them, and sends the frames to the matrix with the RMT peripheral. It also serves the dashboard and its API.
- 💡 **72x12 LED matrix** is a single chain of 864 WS2812B-compatible LEDs, made of two halves rotated 180° from each other.
- 📱 **Nuxt web app** is the dashboard. It is built as a static site and embedded in the firmware, so the matrix needs nothing else on your network.

### 🔄 Data Flow

1. Every 33 ms the player asks the current scene to draw a frame, and blends in the next scene during a crossfade.
2. The output stage maps every pixel to its LED in the chain, applies gamma and brightness, and converts it to GRB bytes.
3. It estimates the current of the frame, and scales the whole frame down if it would go over the power limit.
4. The RMT peripheral sends the bytes to the matrix in the background, while the web server answers in its own task.
5. When you change a setting on the dashboard, the firmware checks every field, saves the settings, and the player picks them up on its next frame.

<a id="quick-start"></a>
## 🚀 Quick Start

### 📋 Requirements

- An ESP32 DevKit with an ESP-WROOM-32 (4 MB flash, the firmware is built for it, env `release`).
- A 72x12 WS2812B matrix, or any 864-LED chain wired the same way.
- A 5 V power supply sized for the brightness you want (see [Power](#power)).
- VS Code with the PlatformIO IDE extension installed, or PlatformIO Core installed locally.
- Node.js 24.21 or newer, to build the web app.

### 🔧 Initial Setup

1. **Clone the repository.**

```bash
git clone https://github.com/brumaombra/holiday-strip.git
```

2. **Build the web app.**

From `webapp/`, install the exact dependencies of the lockfile and build the embedded dashboard:

```bash
npm ci
npm run build:embedded
```

This runs the Nuxt static build, then the `gizmo-embed` tool from [gizmo-common](https://github.com/open-gizmo/gizmo-common) turns it into gzipped C++ files in `microcontroller/src/webui/`. Re-run it any time you change the web app.

3. **Build and upload the firmware over USB.**

For VS Code, open `microcontroller/` in the PlatformIO panel and click **Upload**.

For the CLI, run:

```bash
pio run --target upload
```

This builds the `release` profile. For the serial log, use `debug` instead (see [Build profiles](#build-profiles)).

4. **Connect the matrix to your Wi-Fi.**

On the first boot there's no saved network, so the matrix opens an open hotspot called `Holiday-Strip`. Join it with your phone, and the setup page opens by itself: pick your network and type its password.

5. **Open the dashboard.**

Once the matrix joins your network, open `http://holiday-strip.local/`, or the IP address of the board.

<a id="hardware"></a>
## 🔌 Hardware

### Pinout

Current pin assignments from the firmware configuration:

- Matrix data: GPIO16 (labelled `16` or `RX2` on most DevKits)

The data line enters the matrix on its left edge. Connect a GND pin of the DevKit to the ground of the matrix's power supply, and never power the matrix through the DevKit. The Wi-Fi antenna is the end of the ESP-WROOM-32 module, past its metal can: keep it away from metal.

### Matrix

- Size: `72x12` pixels, `864` LEDs in one chain.
- Protocol: WS2812B-compatible, `800 kHz`, `GRB` byte order.
- Layout: the bottom half (rows 6–11) runs left to right, top to bottom in each column. The top half (rows 0–5) is the bottom half rotated 180°.

The full protocol, the bit timing, and the chain mapping are in [docs/protocol.md](docs/protocol.md).

<a id="power"></a>
### Power

At full white, 864 LEDs can draw about 50 A. The firmware estimates the current of every frame (20 mA per color channel at full value, plus 1 mA of idle current per LED) and dims the frame when it would go over the power limit.

The default limit is `3000 mA`, and the dashboard can set it between `1000` and `8000 mA`. Set it to about 80% of what your 5 V power supply can deliver. If your supply is smaller than 10 A, lower `MAX_MAX_MILLIAMPS` in [config.cpp](microcontroller/src/config/config.cpp) to match it, so the dashboard can't go above it.

<a id="firmware-overview"></a>
## 🔍 Firmware Overview

### ⚙️ General

The firmware lives in `microcontroller/`, and its entry point is [src/main.cpp](microcontroller/src/main.cpp). It:

- Initializes all modules and loads the saved settings.
- Keeps the Wi-Fi connected.
- Applies the settings changed from the dashboard.
- Asks the player for a new frame every 33 ms.

### 🧩 Firmware Layers

The firmware is split into focused modules under `microcontroller/src/`:

- 💡 `display/` holds the frame buffer and colors, maps pixels to the LED chain, limits the power, and drives the RMT peripheral.
- 🔤 `font/` is the 5x7 font of the greeting.
- 🎬 `scenes/` holds one file per scene, the shared effects (snowfall, stars, sprites), and the holidays with their scenes (`scenes.cpp`).
- ▶️ `player/` follows the holiday, rotates its scenes, crossfades between them, and sends the frames to the display.
- 📅 `calendar/` works out the holiday of the date, the Christmas countdown, the time to midnight on New Year's Eve and the year of the fireworks.
- 💾 `settings/` loads and saves the settings in NVS, shares them safely between the loop and the web server, and checks the JSON from the dashboard.
- 🌐 `server/` serves the dashboard and its API, one file per endpoint.
- 📡 `wifi/` runs the Wi-Fi setup and the captive portal from the shared [gizmo-common](https://github.com/open-gizmo/gizmo-common) library, and sets the clock from NTP.
- ⚙️ `config/` centralizes pins, timing values, network names, power figures, and defaults.

### 🌐 Web API

Every response has the OpenGizmo shape `{ errorId, errorMessage, data }`, and only requests from the dashboard's own page are accepted:

- `GET /api/state` returns the settings (with one message per holiday in `messages`), the holidays and their scenes (`holidays`, each with an `id`, a `name` and its `scenes`), the holiday and the scene on screen (`activeHoliday`, `currentScene`), the estimated current, the limits, the uptime and the saved network.
- `POST /api/settings` changes any of `on`, `brightness`, `holiday`, `scene`, `maxMilliamps`, `countdown`, `messages`, `messageColor` (0 to 6: white, warm white, red, green, blue, gold, rainbow, for the custom message) and `language` (`it` or `en`, the texts on the matrix; the greetings still at their default follow it), and answers with the new state. A holiday of `-1` follows the calendar, and a scene of `-1` rotates all the scenes of the holiday. Another holiday without a scene rotates its scenes; a scene chosen while the calendar picks the holiday keeps the holiday playing now. `messages` has one entry per holiday, `null` to leave one as it is. A field with the wrong type or out of range is refused with `invalid_settings_field`, and nothing changes.
- `GET /api/frame` returns the raw RGB bytes of the last frame (72x12, row by row, before gamma and brightness), for the live preview.
- `GET /api/networks`, `POST /api/connect` and `GET /api/check-connection` are the Wi-Fi setup API of gizmo-common.

### 🧪 Tests

The hardware-free firmware logic is tested on your computer, no board needed. From `microcontroller/`:

```bash
pio test -e native
```

The tests live in [microcontroller/test/](microcontroller/test/). They build the logic modules against a small fake `Arduino.h`, and cover:

- The LED chain mapping, checked against the corners measured in [docs/protocol.md](docs/protocol.md).
- The byte order, the gamma, and the power limiter.
- The Christmas countdown, in the real timezone, including the day the clocks go back.
- The holiday of each date, the seconds to midnight on New Year's Eve, and the New Year moment.
- The rules of the saved settings, and the checks of the JSON the dashboard sends.

The web app has its own tests. From `webapp/`:

```bash
npm test
```

<a id="scene-videos"></a>
### 🎬 Scene videos

`microcontroller/tools/scene_video/render.sh` records any scene to an MP4, with the real scene code built for your computer (it needs `g++` and `ffmpeg`). Give it the holiday and the scene by index (in the order of `scenes.cpp`), the seconds to record, the output file, and optionally the local date and time the scenes read:

```bash
microcontroller/tools/scene_video/render.sh 1 0 24 countdown.mp4 2026-12-31T23:59:47
```

This records the New Year's Eve countdown from 13 seconds before midnight. `MESSAGE_COLOR=6` in front of the command records the custom message (holiday 2) in rainbow colors. Without a date, the scenes see a clock that isn't set yet, like a board before it's online.

### 📱 Web App Overview

The dashboard lives in `webapp/`. It's a Nuxt 4 app with the same structure, libraries and versions as the other OpenGizmo web apps, and their pages in the same order (the strip, then Device, then WiFi):

- **Strip** (`/strip`, where the app opens): switch the matrix on and off, set the brightness, and watch the live preview. In the Holiday group, pick a holiday or the calendar. Under it, a group named after the holiday shown (the one playing, while the calendar picks) holds its settings: its scene or the rotation, its custom message (edited in a sheet), for Christmas the countdown, and for the custom message its text color.
- **Device**: the power limit, then the device card of gizmo-common (info, network, restart and forget network).
- **WiFi**: change the network, and the setup page the hotspot opens.

The language (English or Italian) is picked in the top bar, like in every OpenGizmo app. The strip follows it: the dashboard sends it when it changes, and the Christmas countdown and the greetings still at their default (`BUON NATALE!` or `MERRY CHRISTMAS!`, and so on) switch language on the matrix. A greeting you wrote yourself never changes. A new board starts in Italian.

`npm run dev` starts it with a mock server that behaves like the matrix, so you can work on it without a board.

<a id="configuration"></a>
## ⚙️ Configuration

### Build profiles

[platformio.ini](microcontroller/platformio.ini) has the same three profiles as every OpenGizmo firmware:

- `release`, the default: the firmware the release workflow builds and the website flashes, with the serial log off.
- `debug`: the same firmware with the serial log on, and the ESP32 core's own messages. Flash it with `pio run -e debug --target upload`, then open `pio device monitor`.
- `native`: the tests of the hardware-free logic, on your computer (`pio test -e native`).

Nothing needs a secrets file: the Wi-Fi is set from the phone and saved on the board.

### Key runtime settings

The main shared constants live in [config.h](microcontroller/src/config/config.h) and [config.cpp](microcontroller/src/config/config.cpp), including:

- The hostname, the name of the setup hotspot, and the timezone.
- The data pin.
- The frame time, the scene time, and the crossfade time.
- The power figures of the LEDs, and the highest power limit the dashboard can set.
- The default settings, and the longest message.

### Partitions

The firmware uses the `huge_app` partition table, like every OpenGizmo firmware: one 3 MB app slot, with plenty of room for the embedded dashboard. The saved settings and Wi-Fi sit at the same place in every table, so they survive a change of table, which needs one USB upload.

### Releasing a new version

Versions follow [semantic versioning](https://semver.org) (`vMAJOR.MINOR.PATCH`), in the `VERSION` file: a bug fix bumps the patch (1.0.1), a new feature that keeps the saved settings bumps the minor (1.1.0), and a change that resets the settings or needs new wiring bumps the major (2.0.0).

1. On `feature`, set the new version in `VERSION` and in `webapp/package.json` (`npm version X.Y.Z --no-git-tag-version` in `webapp/` updates its lockfile too).
2. Bring `main` up to `feature`.
3. Run the **Release 📦** workflow by hand (Actions > Release > Run workflow, on `main`). It checks the version is new and matches the web app, runs the web app and native tests, and creates the GitHub Release `vX.Y.Z` with notes from the commits. Every firmware build reads the version from `VERSION` (through `microcontroller/firmware_version.py`) and reports it in `/api/device`, so flash the release from its tag.

<a id="troubleshooting"></a>
## 🧰 Troubleshooting

- Nothing lights up: check the data wire on `GPIO13`, and that the ESP32 and the matrix share the same ground.
- Wrong colors or flickering pixels: the data signal is probably too weak or too long. Keep the data wire short, or add a 3.3 V to 5 V level shifter.
- The picture looks dim with bright scenes: the power limiter is dimming it. Raise the limit on the dashboard only if your power supply can deliver it.
- The `Holiday-Strip` hotspot doesn't appear: it only opens when there's no saved network or the saved one fails to connect. Flash the `debug` profile and check the serial monitor.
- `holiday-strip.local` doesn't open: some networks and devices don't support mDNS. Use the IP address of the board, printed on the serial monitor once it connects (with the `debug` profile).
- The dashboard shows an old version: rebuild the web app with `npm run build:embedded` before building the firmware.
- No countdown: the clock is set from NTP, so the countdown only appears once the board is online, and only in the 60 days before Christmas.

<a id="license"></a>
## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
