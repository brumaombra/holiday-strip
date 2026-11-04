#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// A brick fireplace with a crackling fire and stockings hanging from the mantel
	class FireplaceScene : public Scene {
	public:
		// Get the name of the scene
		const char *name() const override { return "Fireplace"; }

		// Cool the whole fire down
		void begin() override { memset(heat, 0, sizeof(heat)); }

		// Draw the brick wall, the fire, the logs, the mantel and the stockings
		void draw(Frame &f, float t, float dt) override {
			// Draw the brick wall
			drawBricks(f);

			// Darken the opening of the fireplace (one pixel wider than the fire on each side)
			for (int y = FIRE_TOP - 1; y < HEIGHT; y++) {
				for (int x = FIRE_X - 1; x < FIRE_X + FIRE_W + 1; x++) {
					setPixel(f, x, y, {8, 3, 2});
				}
			}

			// Advance the fire simulation
			updateFire();

			// Draw the hot cells from the bottom up, above the logs (the black ones are skipped)
			for (int x = 0; x < FIRE_W; x++) {
				for (int h = 0; h < FIRE_H; h++) {
					Color c = heatColor(heat[x][h]);
					if (c.r || c.g || c.b) {
						setPixel(f, FIRE_X + x, HEIGHT - 2 - h, c);
					}
				}
			}

			// Draw the glowing logs under the fire
			drawLogs(f, t);

			// Mantel
			for (int x = 0; x < WIDTH; x++) {
				setPixel(f, x, 0, {90, 45, 20});
				setPixel(f, x, 1, {60, 28, 10});
			}

			// Hang four stockings from the mantel, alternating red and green
			const int stockingsX[] = {3, 10, 57, 64}; // Left column of each stocking
			for (int i = 0; i < 4; i++) {
				drawStocking(f, stockingsX[i], i % 2 ? GREEN : Color{210, 0, 0}, t, i);
			}
		}

	private:
		static constexpr int FIRE_X = 19, FIRE_W = 34, FIRE_TOP = 3; // Left column, width and top row of the fire
		static constexpr int FIRE_H = HEIGHT - 1 - FIRE_TOP;         // Rows of the fire, down to the row above the logs
		uint8_t heat[FIRE_W][FIRE_H];                                // Heat of each cell from 0 to 255 (h = 0 is the bottom row)

		// Get the color of a heat value (black, red, orange, yellow, then white)
		static Color heatColor(uint8_t h) {
			// Scale the heat to 0-191 (three color ramps of 64 steps)
			uint8_t t192 = h * 191 / 255;

			// Get the position inside the current ramp, from 0 to 252
			uint8_t ramp = (t192 & 63) << 2;

			// Hottest: yellow fading to white
			if (t192 >= 128) {
				return {255, 255, ramp};
			}

			// Medium: red fading to yellow
			if (t192 >= 64) {
				return {255, ramp, 0};
			}

			// Coolest: black fading to red
			return {ramp, 0, 0};
		}

		// Cool the cells, spread the heat upwards and ignite new sparks at the bottom
		void updateFire() {
			for (int x = 0; x < FIRE_W; x++) {
				// Flames are taller in the middle of the fireplace
				float center = 1.0f - fabsf(x - FIRE_W / 2.0f) / (FIRE_W / 2.0f);

				// Get the cooling of the column (the middle cools less)
				int cooling = 110 - (int)(center * 45);

				// Cool each cell of the column
				for (int h = 0; h < FIRE_H; h++) {
					// Cool faster higher up so the flames get pointed tips
					int cool = esp_random() % (cooling * (5 + h * 2) / 10 + 1);
					heat[x][h] = heat[x][h] > cool ? heat[x][h] - cool : 0;
				}

				// Spread the heat upwards, mixing the cells below with their left and right neighbors
				for (int h = FIRE_H - 1; h >= 2; h--) {
					int left = heat[max(0, x - 1)][h - 1];
					int right = heat[min(FIRE_W - 1, x + 1)][h - 1];
					heat[x][h] = (heat[x][h - 1] * 2 + heat[x][h - 2] + left + right) / 5;
				}

				// Ignite a new spark at the bottom (more likely in the middle)
				if (esp_random() % 100 < 25 + center * 35) {
					int h = esp_random() % 2;                                           // One of the two bottom rows
					heat[x][h] = min(255, heat[x][h] + 160 + (int)(esp_random() % 96)); // Add 160 to 255 of heat
				}
			}
		}

		// Draw the brick wall below the mantel
		static void drawBricks(Frame &f) {
			// Draw the wall pixel by pixel, from row 2 down
			for (int y = 2; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					// Check if the pixel is mortar (every other row, plus joints shifted on alternate brick rows)
					bool mortar = y % 2 == 1 || (x + (y / 2 % 2) * 3) % 6 == 0;

					// Vary the brick color a little from brick to brick
					Color brick = ((x / 6 + y) % 3 == 0) ? Color{60, 16, 8} : Color{48, 13, 6};

					// Draw the mortar or the brick
					f[y][x] = mortar ? Color{14, 12, 11} : brick;
				}
			}
		}

		// Draw the glowing embers of the logs along the bottom row
		static void drawLogs(Frame &f, float t) {
			// Draw the logs, leaving a two pixel margin on each side of the fire
			for (int x = FIRE_X + 2; x < FIRE_X + FIRE_W - 2; x++) {
				// Get the glow of the ember (it pulses, offset per column)
				float ember = 0.5f + 0.5f * sinf(t * 3.0f + x * 1.7f);

				// Draw the log between dark brown and glowing orange
				setPixel(f, x, HEIGHT - 1, blendColor({70, 30, 10}, {255, 60, 0}, ember * 0.6f));
			}
		}

		// Draw a stocking with a white cuff and a sparkle now and then
		static void drawStocking(Frame &f, int x, Color body, float t, int index) {
			// Define the shape of the stocking ('w' is the cuff, 'r' the body)
			static const char *const SHAPE[] = {"www..", "rrr..", "rrr..", "rrr..", "rrrrr", ".rrrr"};

			// Draw the shape pixel by pixel
			for (int r = 0; r < 6; r++) {
				for (int c = 0; c < 5; c++) {
					char ch = SHAPE[r][c];

					// Skip the transparent pixels
					if (ch == '.') {
						continue;
					}

					// Pick the color (white for the cuff and for the stripe on row 2)
					Color color = ch == 'w' ? WHITE : (r == 2 ? WHITE : body);
					setPixel(f, x + c, 2 + r, color);
				}
			}
			// A little sparkle on each stocking
			if (twinkle(t, 2.0f, index * 1.9f) > 0.92f) {
				setPixel(f, x + 1, 3, GOLD);
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getFireplaceScene() {
	static FireplaceScene scene;
	return scene;
}