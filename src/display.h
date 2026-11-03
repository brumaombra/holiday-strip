#pragma once

#include <Arduino.h>

constexpr int WIDTH = 72;
constexpr int HEIGHT = 12;

struct Color {
  uint8_t r, g, b;
};

typedef Color Frame[HEIGHT][WIDTH];

namespace display {
void begin();
// Brightness 0-255 on top of gamma correction; total current is capped at maxMilliamps
void show(const Frame &frame, uint8_t brightness, uint32_t maxMilliamps);
uint32_t estimatedMilliamps();
}  // namespace display

inline Color scaleColor(Color c, float amount) {
  amount = constrain(amount, 0.0f, 1.0f);
  return {(uint8_t)(c.r * amount), (uint8_t)(c.g * amount), (uint8_t)(c.b * amount)};
}

// amount 0 returns a, 1 returns b
inline Color blendColor(Color a, Color b, float amount) {
  amount = constrain(amount, 0.0f, 1.0f);
  return {(uint8_t)(a.r + (b.r - a.r) * amount), (uint8_t)(a.g + (b.g - a.g) * amount),
          (uint8_t)(a.b + (b.b - a.b) * amount)};
}

inline void setPixel(Frame &f, int x, int y, Color c) {
  if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) f[y][x] = c;
}

inline void addPixel(Frame &f, int x, int y, Color c) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  Color &p = f[y][x];
  p.r = min(255, p.r + c.r);
  p.g = min(255, p.g + c.g);
  p.b = min(255, p.b + c.b);
}

inline void fillFrame(Frame &f, Color c) {
  for (auto &row : f)
    for (auto &p : row) p = c;
}

// Hue 0-255 around the color wheel
inline Color hsvColor(uint8_t h, uint8_t s, uint8_t v) {
  uint8_t region = h / 43;
  uint8_t rem = (h - region * 43) * 6;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * rem) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - rem)) >> 8))) >> 8;
  switch (region) {
    case 0: return {v, t, p};
    case 1: return {q, v, p};
    case 2: return {p, v, t};
    case 3: return {p, q, v};
    case 4: return {t, p, v};
    default: return {v, p, q};
  }
}
