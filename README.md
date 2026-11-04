<div align="center">

# 🎄 Holiday Strip

### An ESP32-powered Christmas show for a 72x12 LED matrix

Holiday Strip is a small ESP32 project that turns a 72x12 truck LED matrix into an animated Christmas display. It rotates through nine hand-drawn scenes with smooth crossfades, scrolls your own greeting with a countdown to Christmas, and comes with a phone-friendly web page to control it all.

<p>
  <img alt="ESP32" src="https://img.shields.io/badge/esp32-ESP32-E7352C?logo=espressif&logoColor=white">
  <img alt="PlatformIO" src="https://img.shields.io/badge/platformio-PlatformIO-FF6B35?logo=platformio&logoColor=white">
  <img alt="Arduino" src="https://img.shields.io/badge/framework-Arduino-00979D?logo=arduino&logoColor=white">
  <img alt="WS2812B" src="https://img.shields.io/badge/leds-864%20WS2812B-16A34A">
  <img alt="Matrix 72x12" src="https://img.shields.io/badge/display-72x12%20matrix-0F766E">
  <img alt="License MIT" src="https://img.shields.io/badge/license-MIT-2563EB">
</p>

<p>
  🎅 Nine animated scenes • 💬 Scrolling greeting • ⏳ Christmas countdown • 📱 Web control • 🔌 Power limiter • 📡 OTA updates
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

This project started from a 72x12 LED matrix pulled from a truck sign. Its protocol was worked out by driving it from an ESP32 and watching it with an ESP32-CAM (see [PROTOCOL.md](PROTOCOL.md)), and now it shows Christmas scenes.

The firmware runs on an ESP32 and:

- Rotates through the scenes every 20 seconds, with a 1-second crossfade.
- Scrolls your message, and counts the days to Christmas from late October.
- Serves a web page on your network to switch it on and off, set the brightness, pick a scene, and edit the message.
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
- Web control page at `http://christmas-matrix.local/`, plus a small JSON API.
- Settings saved in flash, so they survive a reboot.
- Gamma-corrected brightness and a per-frame power limiter.
- Wi-Fi with automatic reconnection, mDNS, and NTP time.
- Over-the-air updates, so the matrix can stay up on the wall.
- WiFi credentials managed through `platformio_secrets.ini`.
- Host tests for the LED mapping, the power limiter, the countdown, and the settings rules.

<a id="architecture"></a>
## 🏗️ Architecture

Holiday Strip is built around three cooperating parts:

- 🔌 **ESP32 firmware** draws the scenes into a frame buffer, mixes them, and sends the frames to the matrix with the RMT peripheral.
- 💡 **72x12 LED matrix** is a single chain of 864 WS2812B-compatible LEDs, made of two halves rotated 180° from each other.
- 📱 **Web page** is served by the ESP32 itself, and talks to it through the JSON API.

### 🔄 Data Flow

1. Every 33 ms the player asks the current scene to draw a frame, and blends in the next scene during a crossfade.
2. The output stage maps every pixel to its LED in the chain, applies gamma and brightness, and converts it to GRB bytes.
3. It estimates the current of the frame, and scales the whole frame down if it would go over the power limit.
4. The RMT peripheral sends the bytes to the matrix in the background, while the loop keeps serving the web page.
5. When you change a setting on the web page, the firmware checks and saves it, and the player picks it up on the next frame.

<a id="quick-start"></a>
## 🚀 Quick Start

### 📋 Requirements

- ESP32 development board (the firmware is built for `esp32dev`).
- A 72x12 WS2812B matrix, or any 864-LED chain wired the same way.
- A 5 V power supply sized for the brightness you want (see [Power](#power)).
- VS Code with the PlatformIO IDE extension installed, or PlatformIO Core installed locally.

### 🔧 Initial Setup

1. **Clone the repository.**

```bash
git clone https://github.com/brumaombra/holiday-strip.git
```

2. **Create your local secrets file.**

```bash
copy platformio_secrets.example.ini platformio_secrets.ini
```

3. **Fill in the following values.**

- Your WiFi SSID and password.
- The serial port of the board (`upload_port` and `monitor_port`).
- The IP address of the board, for over-the-air updates.

4. **Build the firmware.**

For VS Code, open the PlatformIO panel and click **Build**.

For the CLI, run:

```bash
pio run
```

5. **Upload to the ESP32 over USB.**

For VS Code, open the PlatformIO panel and click **Upload**.

For the CLI, run:

```bash
pio run -e matrix --target upload
```

6. **Open the control page.**

Once the board joins your network, open `http://christmas-matrix.local/`, or the IP address of the board.

7. **Update over the air afterward.**

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

The full protocol, the bit timing, and the chain mapping are in [PROTOCOL.md](PROTOCOL.md).

<a id="power"></a>
### Power

At full white, 864 LEDs can draw about 50 A. The firmware estimates the current of every frame (20 mA per color channel at full value, plus 1 mA of idle current per LED) and dims the frame when it would go over the power limit.

The default limit is `3000 mA`. Set it to about 80% of what your 5 V power supply can deliver.

<a id="firmware-overview"></a>
## 🔍 Firmware Overview

### ⚙️ General

The main firmware entry point is [src/main.cpp](src/main.cpp). It:

- Initializes all modules.
- Loads the saved settings.
- Keeps the WiFi connection, the web server, and OTA running.
- Asks the player for a new frame every 33 ms.

### 🧩 Firmware Layers

The firmware is split into focused modules under `src/`:

- 💡 `display/` holds the frame buffer and colors, maps pixels to the LED chain, limits the power, and drives the RMT peripheral.
- 🔤 `font/` is the 5x7 font of the greeting.
- 🎬 `scenes/` holds one file per scene, the shared effects (snowfall, stars, sprites), and the list of scenes.
- ▶️ `player/` rotates the scenes, crossfades between them, and sends the frames to the display.
- 📅 `calendar/` works out the Christmas countdown and the year of the fireworks.
- 💾 `settings/` loads and saves the settings in NVS, and keeps every value in its allowed range.
- 🌐 `web/` serves the control page and the JSON API.
- 📡 `wifi/` connects to the network, sets the clock from NTP, and announces the board with mDNS.
- 🔄 `ota/` handles over-the-air updates, and blanks the matrix while one runs.
- ⚙️ `config/` centralizes pins, timing values, credentials, power figures, and defaults.

### 🌐 Web API

The control page uses a small JSON API:

- `GET /api/state` returns the current settings, the current scene, and the estimated current.
- `GET /api/set?on=1&brightness=80&scene=-1&message=...&countdown=1&max_milliamps=3000` changes one or more settings. A scene of `-1` rotates all of them.
- `GET /api/frame` returns the raw RGB bytes of the last frame (72x12, row by row, before gamma and brightness).

### 🧪 Tests

The hardware-free logic is tested on your computer, no board needed:

```bash
pio test -e native
```

The tests live in [test/](test/). They build the logic modules against a small fake `Arduino.h` ([test/stubs/Arduino.h](test/stubs/Arduino.h)), and cover:

- The LED chain mapping, checked against the corners measured in [PROTOCOL.md](PROTOCOL.md).
- The byte order, the gamma, and the power limiter.
- The Christmas countdown, in the real timezone, including the day the clocks go back.
- The range checks of the settings and the message.

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
	-D PIO_WIFI_SSID='"your-wifi-ssid"'
	-D PIO_WIFI_PASSWORD='"your-wifi-password"'
upload_port = COM5
monitor_port = COM5

[env:matrix-ota]
upload_port = 192.168.1.100
```

Use [platformio_secrets.example.ini](platformio_secrets.example.ini) as the template. The real file is ignored by git.

### Key runtime settings

The main shared constants live in [src/config/config.h](src/config/config.h) and [src/config/config.cpp](src/config/config.cpp), including:

- The hostname and the timezone.
- The data pin.
- The frame time, the scene time, and the crossfade time.
- The power figures of the LEDs.
- The default settings, and the limits of the power limit and the message.

<a id="troubleshooting"></a>
## 🧰 Troubleshooting

- Nothing lights up: check the data wire on `GPIO13`, and that the ESP32 and the matrix share the same ground.
- Wrong colors or flickering pixels: the data signal is probably too weak or too long. Keep the data wire short, or add a 3.3 V to 5 V level shifter.
- The picture looks dim with bright scenes: the power limiter is dimming it. Raise the limit on the control page only if your power supply can deliver it.
- `christmas-matrix.local` doesn't open: some networks and devices don't support mDNS. Use the IP address of the board, printed on the serial monitor once it connects.
- No countdown: the clock is set from NTP, so the countdown only appears once the board is online, and only in the 60 days before Christmas.
- OTA upload fails: check the IP address in `platformio_secrets.ini`, and that your computer is on the same network as the board.

<a id="license"></a>
## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
