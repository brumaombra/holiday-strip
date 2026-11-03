# Christmas LED matrix

Firmware for an ESP32 that drives the 72 × 12 truck LED matrix on GPIO13. It shows animated Christmas scenes. See `PROTOCOL.md` for how the matrix is wired and driven.

## Scenes

The scenes rotate every 20 seconds with a 1-second crossfade:

1. **Greeting**: your message scrolls between two chasing garlands. Between November and December 24 it adds a countdown ("MANCANO N GIORNI A NATALE"). On December 25 it shows "OGGI E' NATALE!".
2. **Snowfall**: snow falls and piles up next to a little house with warm windows.
3. **Christmas trees**: six trees with twinkling ornaments and gold stars, in falling snow.
4. **Santa's sleigh**: Santa and two reindeer fly across a starry sky with a moon. Rudolph's nose blinks red, and the sleigh leaves a trail of sparkles.
5. **Fireplace**: a brick fireplace with a simulated fire and glowing logs. Red and green stockings hang from the mantel.
6. **Fairy lights**: two strings of lights hanging in swags. They alternate between twinkle, chase and color-cycle effects.
7. **Gifts**: wrapped presents with ribbons and bows fall and stack up. When the pile gets too high, it sinks away and starts over.
8. **Candy cane**: shaded red and white stripes sliding along, with sparkles.
9. **Fireworks**: rockets launch and burst into colored sparks over a dim gold year number. From November on, it shows the coming year.

## Web control

Open `http://christmas-matrix.local/`, or the board's IP address, currently `192.168.21.142`. From there you can:

- turn the display on or off
- set the brightness
- show one scene or rotate through all of them
- edit the scrolling message (letters A–Z, digits and `! ' * , - . : ?`; `<` draws a heart)
- turn the countdown on or off
- set the power limit

The settings are saved in flash and survive a reboot.

The same controls are available as an API: `GET /api/state`, and `GET /api/set?on=1&brightness=80&scene=-1&message=...&countdown=1&max_milliamps=3000`.

## Power

Brightness goes through gamma correction. The firmware estimates the current draw of every frame and dims the frame if it would go over the power limit. The default limit is 3000 mA. Set it to about 80% of what your 5 V power supply can deliver.

## Building and flashing

- USB: `pio run -e matrix -t upload` (COM5)
- WiFi (OTA): `pio run -e matrix-ota -t upload`. This uses the IP set in `platformio.ini`.

WiFi credentials go in `include/secrets.h`. See `include/secrets.example.h` for the format.
