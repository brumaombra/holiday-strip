#pragma once

#include <Arduino.h>

// Size of the matrix, colors, frames and drawing helpers, kept free of the hardware so the native tests can use them

constexpr int WIDTH = 72;
constexpr int HEIGHT = 12;

// A color, 8 bits per channel
struct Color {
	uint8_t r, g, b;
};

// A frame of the whole matrix, row by row
typedef Color Frame[HEIGHT][WIDTH];

// Scale a color, 0 is black and 1 the color itself
inline Color scaleColor(Color c, float amount) {
	amount = constrain(amount, 0.0f, 1.0f);
	return {(uint8_t)(c.r * amount), (uint8_t)(c.g * amount), (uint8_t)(c.b * amount)};
}

// Blend two colors: amount 0 returns a, 1 returns b
inline Color blendColor(Color a, Color b, float amount) {
	amount = constrain(amount, 0.0f, 1.0f);
	return {(uint8_t)(a.r + (b.r - a.r) * amount), (uint8_t)(a.g + (b.g - a.g) * amount),
	        (uint8_t)(a.b + (b.b - a.b) * amount)};
}

// Set a pixel (ignored outside the matrix)
inline void setPixel(Frame &f, int x, int y, Color c) {
	if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
		f[y][x] = c;
	}
}

// Add a color to a pixel, saturating at white (ignored outside the matrix)
inline void addPixel(Frame &f, int x, int y, Color c) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
		return;
	}
	Color &p = f[y][x];
	p.r = min(255, p.r + c.r);
	p.g = min(255, p.g + c.g);
	p.b = min(255, p.b + c.b);
}

// Fill a frame with one color
inline void fillFrame(Frame &f, Color c) {
	for (auto &row : f) {
		for (auto &p : row) {
			p = c;
		}
	}
}

// Get a color from hue, saturation and value (hue 0-255 around the color wheel)
inline Color hsvColor(uint8_t h, uint8_t s, uint8_t v) {
	uint8_t region = h / 43;
	uint8_t rem = (h - region * 43) * 6;
	uint8_t p = (v * (255 - s)) >> 8;
	uint8_t q = (v * (255 - ((s * rem) >> 8))) >> 8;
	uint8_t t = (v * (255 - ((s * (255 - rem)) >> 8))) >> 8;
	switch (region) {
		case 0:
			return {v, t, p};
		case 1:
			return {q, v, p};
		case 2:
			return {p, v, t};
		case 3:
			return {p, q, v};
		case 4:
			return {t, p, v};
		default:
			return {v, p, q};
	}
}