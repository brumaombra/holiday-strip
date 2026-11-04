#pragma once

#include <Arduino.h>

#include "display/frame.h"

// Colors, sprites and effects shared by the scenes

constexpr Color RED = {255, 0, 0};
constexpr Color GREEN = {0, 200, 30};
constexpr Color GOLD = {255, 150, 0};
constexpr Color WHITE = {255, 255, 255};
constexpr Color WARM_WHITE = {255, 190, 110};
constexpr Color BLUE = {20, 60, 255};
constexpr Color PINK = {255, 40, 120};
constexpr Color SNOW = {200, 205, 225};

constexpr Color LIGHT_COLORS[] = {RED, GREEN, BLUE, GOLD, PINK, WARM_WHITE};
constexpr int LIGHT_COLOR_COUNT = sizeof(LIGHT_COLORS) / sizeof(LIGHT_COLORS[0]);

inline float frand(float lo, float hi) { return lo + (hi - lo) * (esp_random() / 4294967295.0f); }

inline float twinkle(float t, float speed, float phase) { return 0.5f + 0.5f * sinf(t * speed + phase); }

// Sprites: '.' is transparent, other characters map to a palette color
inline Color spriteColor(char c, float t) {
	switch (c) {
		case 'r':
			return {210, 0, 0};
		case 'w':
			return WHITE;
		case 's':
			return {255, 160, 110};
		case 'b':
			return {130, 60, 15};
		case 'a':
			return {200, 150, 70};
		case 'y':
			return GOLD;
		case 'g':
			return GREEN;
		case 'n':
			return scaleColor(RED, 0.3f + 0.7f * twinkle(t, 6.0f, 0)); // Rudolph's nose
		default:
			return {0, 0, 0};
	}
}

inline void drawSprite(Frame &f, const char *const *rows, int rowCount, int x, int y, float t) {
	for (int r = 0; r < rowCount; r++) {
		for (int c = 0; rows[r][c]; c++) {
			if (rows[r][c] != '.') {
				setPixel(f, x + c, y + r, spriteColor(rows[r][c], t));
			}
		}
	}
}

struct Snowfall {
	struct Flake {
		float x, y, speed, phase;
	};
	static constexpr int MAX_FLAKES = 48;
	Flake flakes[MAX_FLAKES];
	int count = 0;

	void begin(int n) {
		count = min(n, MAX_FLAKES);
		for (int i = 0; i < count; i++) {
			respawn(flakes[i], true);
		}
	}

	void respawn(Flake &fl, bool anywhere) {
		fl.x = frand(0, WIDTH);
		fl.y = anywhere ? frand(-HEIGHT, HEIGHT) : frand(-4, -0.5f);
		fl.speed = frand(1.5f, 4.5f);
		fl.phase = frand(0, 6.28f);
	}

	int columnOf(const Flake &fl, float t) const {
		return constrain((int)lroundf(fl.x + sinf(t * 1.3f + fl.phase) * 0.8f), 0, WIDTH - 1);
	}

	// ground: optional per-column snow height that grows as flakes land
	void update(float t, float dt, float *ground) {
		for (int i = 0; i < count; i++) {
			Flake &fl = flakes[i];
			fl.y += fl.speed * dt;
			int x = columnOf(fl, t);
			float limit = HEIGHT - (ground ? ground[x] : 0);
			if (fl.y >= limit) {
				if (ground) {
					ground[x] = min(ground[x] + 0.35f, 4.0f);
				}
				respawn(fl, false);
			}
		}
	}

	void draw(Frame &f, float t) const {
		for (int i = 0; i < count; i++) {
			const Flake &fl = flakes[i];
			if (fl.y < 0) {
				continue;
			}
			uint8_t v = 120 + (fl.speed - 1.5f) / 3.0f * 135;
			addPixel(f, columnOf(fl, t), (int)fl.y, {v, v, v});
		}
	}
};

struct Stars {
	struct Star {
		uint8_t x, y;
		float speed, phase;
	};
	static constexpr int COUNT = 30;
	Star stars[COUNT];

	void begin() {
		for (auto &s : stars) {
			s.x = esp_random() % WIDTH;
			s.y = esp_random() % HEIGHT;
			s.speed = frand(1.0f, 4.0f);
			s.phase = frand(0, 6.28f);
		}
	}

	void draw(Frame &f, float t) const {
		for (const auto &s : stars) {
			float v = twinkle(t, s.speed, s.phase);
			addPixel(f, s.x, s.y, scaleColor({180, 180, 255}, v * v * 0.8f));
		}
	}
};