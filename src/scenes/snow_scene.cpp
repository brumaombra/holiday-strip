#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Snow falls and piles up next to a small house with lit windows
	class SnowScene : public Scene {
	public:
		const char *name() const override { return "Snowfall"; }

		void begin() override {
			snow.begin(40);
			for (auto &g : ground) {
				g = 0;
			}
		}

		void draw(Frame &f, float t, float dt) override {
			for (int y = 0; y < HEIGHT; y++) {
				Color c = blendColor({0, 0, 12}, {0, 6, 35}, y / float(HEIGHT - 1));
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = c;
				}
			}

			drawHouse(f, t);
			snow.update(t, dt, ground);
			settleGround(dt);
			drawGround(f);
			snow.draw(f, t);
		}

	private:
		Snowfall snow;
		float ground[WIDTH];

		void settleGround(float dt) {
			for (int x = 0; x < WIDTH - 1; x++) {
				float diff = ground[x] - ground[x + 1];
				if (fabsf(diff) > 1.0f) {
					float move = diff * 0.5f * dt;
					ground[x] -= move;
					ground[x + 1] += move;
				}
			}
		}

		void drawGround(Frame &f) {
			for (int x = 0; x < WIDTH; x++) {
				int full = (int)ground[x];
				float frac = ground[x] - full;
				for (int i = 0; i < full; i++) {
					setPixel(f, x, HEIGHT - 1 - i, SNOW);
				}
				if (frac > 0.05f) {
					int y = HEIGHT - 1 - full;
					if (y >= 0) {
						f[y][x] = blendColor(f[y][x], SNOW, frac);
					}
				}
			}
		}

		static void drawHouse(Frame &f, float t) {
			const int cx = 57;
			const Color wall = {110, 45, 15};
			const Color roof = {150, 15, 15};
			// Chimney
			for (int y = 2; y <= 4; y++) {
				setPixel(f, 61, y, {120, 40, 30});
			}
			setPixel(f, 61, 1, SNOW);
			// Roof with snow on its top edge
			for (int y = 3; y <= 6; y++) {
				int half = 1 + (y - 3) * 2;
				int halfAbove = y == 3 ? -1 : 1 + (y - 4) * 2;
				for (int x = cx - half; x <= cx + half; x++) {
					bool top = abs(x - cx) > halfAbove;
					setPixel(f, x, y, top ? SNOW : roof);
				}
			}
			for (int y = 7; y < HEIGHT; y++) {
				for (int x = cx - 6; x <= cx + 6; x++) {
					setPixel(f, x, y, wall);
				}
			}
			// Door and warm flickering windows
			for (int y = 9; y < HEIGHT; y++) {
				setPixel(f, cx, y, {50, 20, 5});
			}
			float flicker = 0.8f + 0.2f * sinf(t * 7.0f) * sinf(t * 2.3f);
			Color light = scaleColor({255, 170, 50}, flicker);
			for (int y = 8; y <= 9; y++) {
				setPixel(f, cx - 4, y, light);
				setPixel(f, cx - 3, y, light);
				setPixel(f, cx + 3, y, light);
				setPixel(f, cx + 4, y, light);
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getSnowScene() {
	static SnowScene scene;
	return scene;
}