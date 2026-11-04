#ifndef EFFECTS_H
#define EFFECTS_H

#include <Arduino.h>

#include "display/frame.h"

// Colors, sprites and effects shared by the scenes

constexpr Color RED = { 255, 0, 0 };
constexpr Color GREEN = { 0, 200, 30 };
constexpr Color GOLD = { 255, 150, 0 };
constexpr Color WHITE = { 255, 255, 255 };
constexpr Color WARM_WHITE = { 255, 190, 110 };
constexpr Color BLUE = { 20, 60, 255 };
constexpr Color PINK = { 255, 40, 120 };
constexpr Color SNOW = { 200, 205, 225 };

// Colors the lights and ornaments cycle through
constexpr Color LIGHT_COLORS[] = { RED, GREEN, BLUE, GOLD, PINK, WARM_WHITE };
constexpr int LIGHT_COLOR_COUNT = sizeof(LIGHT_COLORS) / sizeof(LIGHT_COLORS[0]); // Number of light colors

// Get a random float between lo and hi
inline float frand(float lo, float hi) { return lo + (hi - lo) * (esp_random() / 4294967295.0f); }

// Get a brightness between 0 and 1 that pulses over time (the phase offsets each light from the others)
inline float twinkle(float t, float speed, float phase) { return 0.5f + 0.5f * sinf(t * speed + phase); }

// Get the color of a sprite character: '.' is transparent, the others map to the palette (t animates Rudolph's nose)
inline Color spriteColor(char c, float t) {
	switch (c) {
		case 'r':
			return { 210, 0, 0 }; // Red
		case 'w':
			return WHITE;
		case 's':
			return { 255, 160, 110 }; // Skin
		case 'b':
			return { 130, 60, 15 }; // Dark brown
		case 'a':
			return { 200, 150, 70 }; // Light brown (antlers)
		case 'y':
			return GOLD;
		case 'g':
			return GREEN;
		case 'n':
			return scaleColor(RED, 0.3f + 0.7f * twinkle(t, 6.0f, 0)); // Rudolph's nose
		default:
			return { 0, 0, 0 };
	}
}

// Draw a sprite at x, y (the rows are strings of palette characters)
inline void drawSprite(Frame &f, const char *const *rows, int rowCount, int x, int y, float t) {
	// Go through the sprite row by row
	for (int r = 0; r < rowCount; r++) {
		for (int c = 0; rows[r][c]; c++) {
			// Draw the pixel unless it's transparent
			if (rows[r][c] != '.') {
				setPixel(f, x + c, y + r, spriteColor(rows[r][c], t));
			}
		}
	}
}

// A field of falling snowflakes, shared by the scenes with snow
struct Snowfall {
	// A single snowflake
	struct Flake {
		float x, y, speed, phase; // Position, fall speed (rows per second) and sway phase
	};
	static constexpr int MAX_FLAKES = 48; // Size of the flake pool
	Flake flakes[MAX_FLAKES];
	int count = 0; // Number of flakes in use

	// Start n flakes spread over and above the screen (at most the size of the pool)
	void begin(int n) {
		// Limit the flakes to the size of the pool
		count = min(n, MAX_FLAKES);

		// Place each flake at a random spot
		for (int i = 0; i < count; i++) {
			respawn(flakes[i], true);
		}
	}

	// Put a flake back at the top (anywhere also spreads it over the screen, for the start)
	void respawn(Flake &fl, bool anywhere) {
		fl.x = frand(0, WIDTH);
		fl.y = anywhere ? frand(-HEIGHT, HEIGHT) : frand(-4, -0.5f); // Start above the screen so the flakes don't all appear at once
		fl.speed = frand(1.5f, 4.5f); // Rows per second
		fl.phase = frand(0, 6.28f); // Up to a full turn of the sway
	}

	// Get the column of a flake, swaying left and right as it falls
	int columnOf(const Flake &fl, float t) const {
		return constrain((int)lroundf(fl.x + sinf(t * 1.3f + fl.phase) * 0.8f), 0, WIDTH - 1); // Sway by 0.8 columns
	}

	// Move the flakes down and respawn the ones that land (ground: optional snow height per column, it grows as flakes land)
	void update(float t, float dt, float *ground) {
		for (int i = 0; i < count; i++) {
			Flake &fl = flakes[i];

			// Move the flake down
			fl.y += fl.speed * dt;

			// Get the lowest row the flake can reach (on top of the snow on the ground)
			int x = columnOf(fl, t);
			float limit = HEIGHT - (ground ? ground[x] : 0);

			// Respawn the flake when it lands (it also grows the snow on the ground)
			if (fl.y >= limit) {
				if (ground) {
					ground[x] = min(ground[x] + 0.35f, 4.0f); // Grow the pile by 0.35 rows, up to 4 rows
				}
				respawn(fl, false);
			}
		}
	}

	// Draw the flakes (the faster ones are brighter, as if closer)
	void draw(Frame &f, float t) const {
		for (int i = 0; i < count; i++) {
			const Flake &fl = flakes[i];

			// Skip the flakes still above the screen
			if (fl.y < 0) {
				continue;
			}

			// Get the brightness from the speed (120 for the slowest, 255 for the fastest)
			uint8_t v = 120 + (fl.speed - 1.5f) / 3.0f * 135;

			// Draw the flake
			addPixel(f, columnOf(fl, t), (int)fl.y, { v, v, v });
		}
	}
};

// A sky of twinkling stars
struct Stars {
	// A single star
	struct Star {
		uint8_t x, y; // Position
		float speed, phase; // Twinkle speed and phase
	};
	static constexpr int COUNT = 30; // Number of stars
	Star stars[COUNT];

	// Place the stars at random positions with random twinkles
	void begin() {
		for (auto &s : stars) {
			// Pick a random position
			s.x = esp_random() % WIDTH;
			s.y = esp_random() % HEIGHT;

			// Pick a random twinkle speed and phase
			s.speed = frand(1.0f, 4.0f);
			s.phase = frand(0, 6.28f);
		}
	}

	// Draw the stars at their current brightness
	void draw(Frame &f, float t) const {
		for (const auto &s : stars) {
			// Get the brightness of the star
			float v = twinkle(t, s.speed, s.phase);

			// Draw the star in pale blue (squared so it stays dim most of the time)
			addPixel(f, s.x, s.y, scaleColor({ 180, 180, 255 }, v * v * 0.8f));
		}
	}
};

#endif