# LED matrix protocol

Worked out by driving GPIO13 from an ESP32 and watching the matrix with the ESP32-CAM.

## Electrical protocol

- WS2812B-compatible one-wire protocol at 800 kHz. The data line is on the left edge of the matrix.
- Bit timing used, all confirmed working:
  - 0 bit: 400 ns high, then 850 ns low
  - 1 bit: 800 ns high, then 450 ns low
  - Latch/reset: line low for 300 µs or more
- 3 bytes per LED in **GRB** order, most significant bit first.
- The LEDs keep showing the last frame until new data arrives.
- A full frame takes about 26 ms to send (864 × 24 bits × 1.25 µs), so the matrix can refresh at up to about 38 fps.

## Geometry

- **72 × 12 pixels, 864 LEDs** in a single chain.
- The matrix is made of two 72 × 6 halves. The top half is the bottom half rotated 180°.

| Half | Chain indices | Column order | Direction within a column |
|---|---|---|---|
| Bottom (rows 6–11) | 0 – 431 | left → right | top → bottom |
| Top (rows 0–5) | 432 – 863 | right → left | bottom → top |

Mapping from screen coordinates (x 0..71 from the left, y 0..11 from the top) to chain index:

```c
int pixelIndex(int x, int y) {
  if (y >= 6) return x * 6 + (y - 6);
  return 432 + (71 - x) * 6 + (5 - y);
}
```

Corner check: index 0 is at (0, 6), index 431 at (71, 11), index 432 at (71, 5), and index 863 at (0, 0).

## Power

At full white, 864 WS2812B LEDs can draw up to about 50 A (60 mA each). Limit brightness in firmware, and size the power supply for the brightest content you plan to show.
