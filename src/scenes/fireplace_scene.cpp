#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// A brick fireplace with a crackling fire and stockings hanging from the mantel
	class FireplaceScene : public Scene {
	public:
		const char *name() const override { return "Fireplace"; }

		void begin() override { memset(heat, 0, sizeof(heat)); }

		void draw(Frame &f, float t, float dt) override {
			drawBricks(f);
			for (int y = FIRE_TOP - 1; y < HEIGHT; y++) {
				for (int x = FIRE_X - 1; x < FIRE_X + FIRE_W + 1; x++) {
					setPixel(f, x, y, {8, 3, 2});
				}
			}

			updateFire();
			for (int x = 0; x < FIRE_W; x++) {
				for (int h = 0; h < FIRE_H; h++) {
					Color c = heatColor(heat[x][h]);
					if (c.r || c.g || c.b) {
						setPixel(f, FIRE_X + x, HEIGHT - 2 - h, c);
					}
				}
			}
			drawLogs(f, t);

			// Mantel
			for (int x = 0; x < WIDTH; x++) {
				setPixel(f, x, 0, {90, 45, 20});
				setPixel(f, x, 1, {60, 28, 10});
			}
			const int stockingsX[] = {3, 10, 57, 64};
			for (int i = 0; i < 4; i++) {
				drawStocking(f, stockingsX[i], i % 2 ? GREEN : Color{210, 0, 0}, t, i);
			}
		}

	private:
		static constexpr int FIRE_X = 19, FIRE_W = 34, FIRE_TOP = 3;
		static constexpr int FIRE_H = HEIGHT - 1 - FIRE_TOP;
		uint8_t heat[FIRE_W][FIRE_H];

		static Color heatColor(uint8_t h) {
			uint8_t t192 = h * 191 / 255;
			uint8_t ramp = (t192 & 63) << 2;
			if (t192 >= 128) {
				return {255, 255, ramp};
			}
			if (t192 >= 64) {
				return {255, ramp, 0};
			}
			return {ramp, 0, 0};
		}

		void updateFire() {
			for (int x = 0; x < FIRE_W; x++) {
				// Flames are taller in the middle of the fireplace
				float center = 1.0f - fabsf(x - FIRE_W / 2.0f) / (FIRE_W / 2.0f);
				int cooling = 110 - (int)(center * 45);
				for (int h = 0; h < FIRE_H; h++) {
					// Cool faster higher up so the flames get pointed tips
					int cool = esp_random() % (cooling * (5 + h * 2) / 10 + 1);
					heat[x][h] = heat[x][h] > cool ? heat[x][h] - cool : 0;
				}
				for (int h = FIRE_H - 1; h >= 2; h--) {
					int left = heat[max(0, x - 1)][h - 1];
					int right = heat[min(FIRE_W - 1, x + 1)][h - 1];
					heat[x][h] = (heat[x][h - 1] * 2 + heat[x][h - 2] + left + right) / 5;
				}
				if (esp_random() % 100 < 25 + center * 35) {
					int h = esp_random() % 2;
					heat[x][h] = min(255, heat[x][h] + 160 + (int)(esp_random() % 96));
				}
			}
		}

		static void drawBricks(Frame &f) {
			for (int y = 2; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					bool mortar = y % 2 == 1 || (x + (y / 2 % 2) * 3) % 6 == 0;
					Color brick = ((x / 6 + y) % 3 == 0) ? Color{60, 16, 8} : Color{48, 13, 6};
					f[y][x] = mortar ? Color{14, 12, 11} : brick;
				}
			}
		}

		static void drawLogs(Frame &f, float t) {
			for (int x = FIRE_X + 2; x < FIRE_X + FIRE_W - 2; x++) {
				float ember = 0.5f + 0.5f * sinf(t * 3.0f + x * 1.7f);
				setPixel(f, x, HEIGHT - 1, blendColor({70, 30, 10}, {255, 60, 0}, ember * 0.6f));
			}
		}

		static void drawStocking(Frame &f, int x, Color body, float t, int index) {
			static const char *const SHAPE[] = {"www..", "rrr..", "rrr..", "rrr..", "rrrrr", ".rrrr"};
			for (int r = 0; r < 6; r++) {
				for (int c = 0; c < 5; c++) {
					char ch = SHAPE[r][c];
					if (ch == '.') {
						continue;
					}
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