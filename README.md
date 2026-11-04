<div align="center">

# 🎄 Holiday Strip

### An ESP32-powered Christmas show for a 72x12 LED matrix

Holiday Strip is a small ESP32 project that turns a 72x12 truck LED matrix into an animated Christmas display. It rotates through nine hand-drawn scenes with smooth crossfades, scrolls your own greeting with a countdown to Christmas, and comes with a phone-friendly dashboard, built into the firmware, to control it all.

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
  🎅 Nine animated scenes • 💬 Scrolling greeting • ⏳ Christmas countdown • 📱 Web dashboard • 🔌 Power limiter • 📡 OTA updates
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

This project started from a 72x12 LED matrix pulled from a truck sign. Its protocol was worked out by driving it from an ESP32 and watching it with an ESP32-CAM (see [docs/protocol.md](docs/protocol.md)), and now it shows Christmas scenes.

The firmware runs on an ESP32 and:

- Rotates through the scenes every 20 seconds, with a 1-second crossfade.
- Scrolls your message, and counts the days to Christmas from late October.
- Serves a dashboard on your network to switch it on and off, set the brightness, watch a live preview, pick a scene, and edit the message.
- Estimates the current of every frame and dims it before it goes over your power supply's limit.

### 🎬 Scenes

1. **Greeting**: your message scrolls between two chasing garlands. In the 60 days before Christmas it adds a countdown (`MANCANO N GIORNI A NATALE!`), and on December 25 it shows `OGGI E' NATALE!`.
2. **Snowfall**: snow falls and piles up next to a little house with warm windows.
3. **Christmas trees**: six trees with twinkling ornaments and gold stars, in falling snow.
4. **Santa's sleigh**: Santa and two reindeer fly across a starry sky with a moon. Rudolph's nose blinks red, and the sleigh leaves a trail of sparkles.
5. **Fireplace**: a brick fireplace with a simulated fire and glowing logs. Red and green stockings hang from the mantel.
6. **Fairy lights**: two strings of lights hanging in swags. They alternate between twinkle, chase, and color-cycle effects.
7. **Gifts**: wrapped presents with ribbons and bows fall and stack up. When the pile gets too high, it sinks away and starts over.
8. **Candy cane**: shaded red and white stripes sliding along, with sparkles.
9. **Fireworks**: rockets launch and burst into colored sparks over a dim gold year number. From November on, it shows the coming year.

<a id="features"></a>
## ✨ Features

- Nine animated scenes at about 30 fps, rotating with crossfades, or one scene shown on its own.
- Scrolling greeting in capitals (letters A–Z, digits, and `! ' * , - . : ?`, while `<` draws a heart).
- Christmas countdown that follows the local timezone, daylight saving time included.
- Web dashboard built into the firmware, with a live preview of the matrix, the scene picker, the message editor and the power limit.
- Wi-Fi setup from your phone: with no saved network, the matrix opens a setup hotspot with a captive portal.
- Settings saved in flash, so they survive a reboot.
- Gamma-corrected brightness and a per-frame power limiter, capped below what the power supply can deliver.
- A JSON API that only answers the dashboard's own page, so other websites can't change the settings from your browser.
- Password-protected over-the-air updates, so the matrix can stay up on the wall.
- Host tests for the LED mapping, the power limiter, the countdown and the settings, and Vitest tests for the web app.

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

- ESP32 development board (the firmware is built for `esp32dev`).
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

3. **Create your local secrets file.**

From `microcontroller/`:

```bash
copy platformio_secrets.example.ini platformio_secrets.ini
```

Fill in the serial port of the board, and choose a password for wireless updates (the same one in both places).

4. **Build and upload the firmware over USB.**

For VS Code, open `microcontroller/` in the PlatformIO panel and click **Upload**.

For the CLI, run:

```bash
pio run -e matrix --target upload
```

5. **Connect the matrix to your Wi-Fi.**

On the first boot there's no saved network, so the matrix opens an open hotspot called `Christmas-Matrix`. Join it with your phone, and the setup page opens by itself: pick your network and type its password.

6. **Open the dashboard.**

Once the matrix joins your network, open `http://christmas-matrix.local/`, or the IP address of the board.

7. **Update over the air afterward.**

Put the IP address of the board in `platformio_secrets.ini`, then run:

```bash
pio run -e matrix-ota --target upload
```

<a id="hardware"></a>
## 🔌 Hardware

### Pinout

Current pin assignments from the firmware configuration:

- Matrix data: `GPIO13`

The data line enters the matrix on its left edge. Connect the ground of the ESP32 to the ground of the matrix's power supply.

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
- Keeps the Wi-Fi connected, and wireless updates running.
- Applies the settings changed from the dashboard.
- Asks the player for a new frame every 33 ms.

### 🧩 Firmware Layers

The firmware is split into focused modules under `microcontroller/src/`:

- 💡 `display/` holds the frame buffer and colors, maps pixels to the LED chain, limits the power, and drives the RMT peripheral.
- 🔤 `font/` is the 5x7 font of the greeting.
- 🎬 `scenes/` holds one file per scene, the shared effects (snowfall, stars, sprites), and the list of scenes.
- ▶️ `player/` rotates the scenes, crossfades between them, and sends the frames to the display.
- 📅 `calendar/` works out the Christmas countdown and the year of the fireworks.
- 💾 `settings/` loads and saves the settings in NVS, shares them safely between the loop and the web server, and checks the JSON from the dashboard.
- 🌐 `server/` serves the dashboard and its API, one file per endpoint.
- 📡 `wifi/` runs the Wi-Fi setup and the captive portal from the shared [gizmo-common](https://github.com/open-gizmo/gizmo-common) library, and sets the clock from NTP.
- 🔄 `ota/` handles password-protected wireless updates, and blanks the matrix while one runs.
- ⚙️ `config/` centralizes pins, timing values, network names, power figures, and defaults.

### 🌐 Web API

Every response has the OpenGizmo shape `{ errorId, errorMessage, data }`, and only requests from the dashboard's own page are accepted:

- `GET /api/state` returns the settings, the scene names, the scene on screen, the estimated current, the limits, the uptime and the saved network.
- `POST /api/settings` changes any of `on`, `brightness`, `scene`, `maxMilliamps`, `countdown` and `message`, and answers with the new state. A scene of `-1` rotates all of them. A field with the wrong type or out of range is refused with `invalid_settings_field`, and nothing changes.
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
- The rules of the saved settings, and the checks of the JSON the dashboard sends.

The web app has its own tests. From `webapp/`:

```bash
npm test
```

### 📱 Web App Overview

The dashboard lives in `webapp/`. It's a Nuxt 4 app with the same structure, libraries and versions as the other OpenGizmo web apps:

- **Home**: switch the matrix on and off, set the brightness, watch the live preview, and pick a scene or the rotation.
- **Message**: edit the scrolling message, and turn the Christmas countdown on or off.
- **Settings**: the power limit, the language (English or Italian), and the device info.
- **Wi-Fi**: change the network, and the setup page the hotspot opens.

`npm run dev` starts it with a mock server that behaves like the matrix, so you can work on it without a board.

<a id="configuration"></a>
## ⚙️ Configuration

### Secrets file

The project uses PlatformIO extra configs:

```ini
[platformio]
extra_configs = platformio_secrets.ini
```

Example secrets file:

```ini
[env:matrix]
build_flags =
	-D PIO_OTA_PASSWORD='"your-ota-password"'
upload_port = COM5
monitor_port = COM5

[env:matrix-ota]
upload_port = 192.168.1.100
upload_flags =
	--auth=your-ota-password
```

Use [platformio_secrets.example.ini](microcontroller/platformio_secrets.example.ini) as the template. The real file is ignored by git. Leave the OTA password empty to turn wireless updates off. The Wi-Fi credentials aren't in the firmware: they're set from the setup hotspot, and saved on the board.

### Key runtime settings

The main shared constants live in [config.h](microcontroller/src/config/config.h) and [config.cpp](microcontroller/src/config/config.cpp), including:

- The hostname, the name of the setup hotspot, and the timezone.
- The data pin.
- The frame time, the scene time, and the crossfade time.
- The power figures of the LEDs, and the highest power limit the dashboard can set.
- The default settings, and the longest message.

### Partitions

The firmware uses the `min_spiffs` partition table: two 1.9 MB app slots, so the embedded dashboard fits and wireless updates still work. Moving from the older default table needs one USB upload. The saved settings are kept.

<a id="troubleshooting"></a>
## 🧰 Troubleshooting

- Nothing lights up: check the data wire on `GPIO13`, and that the ESP32 and the matrix share the same ground.
- Wrong colors or flickering pixels: the data signal is probably too weak or too long. Keep the data wire short, or add a 3.3 V to 5 V level shifter.
- The picture looks dim with bright scenes: the power limiter is dimming it. Raise the limit on the dashboard only if your power supply can deliver it.
- The `Christmas-Matrix` hotspot doesn't appear: it only opens when there's no saved network or the saved one fails to connect. Check the serial monitor.
- `christmas-matrix.local` doesn't open: some networks and devices don't support mDNS. Use the IP address of the board, printed on the serial monitor once it connects.
- The dashboard shows an old version: rebuild the web app with `npm run build:embedded` before building the firmware.
- No countdown: the clock is set from NTP, so the countdown only appears once the board is online, and only in the 60 days before Christmas.
- OTA upload fails with an authentication error: the password in `upload_flags` must match `PIO_OTA_PASSWORD` in the firmware on the board.
- OTA upload doesn't find the board: check the IP address in `platformio_secrets.ini`, and that your computer is on the same network as the board.

<a id="license"></a>
## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
