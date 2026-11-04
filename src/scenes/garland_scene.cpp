#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Two strings of fairy lights hanging in swags, cycling through light effects
	class GarlandScene : public Scene {
	public:
		const char *name() const override { return "Fairy lights"; }

		void begin() override {
			for (auto &p : phases) {
				p = frand(0, 6.28f);
			}
		}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, {0, 0, 0});
			int mode = ((int)(t / 6.0f)) % 3;
			for (int g = 0; g < 2; g++) {
				int base = g == 0 ? 0 : 6;
				float offset = g == 0 ? 0 : 12;
				int bulb = 0;
				for (int x = 0; x < WIDTH; x++) {
					float u = fmodf(x + offset, SWAG) - SWAG / 2;
					int wy = base + (int)lroundf(3.0f * (1.0f - (u * u) / (SWAG * SWAG / 4)));
					setPixel(f, x, wy, {0, 35, 5});
					if (((int)(x + offset)) % 4 != 2) {
						continue;
					}

					int id = g * 20 + bulb++;
					Color c = LIGHT_COLORS[id % LIGHT_COLOR_COUNT];
					float v;
					if (mode == 0) {
						v = 0.15f + 0.85f * powf(twinkle(t, 2.0f, phases[id % 40]), 2);
					} else if (mode == 1) {
						v = ((id + (int)(t * 6)) % 4 == 0) ? 1.0f : 0.15f;
					} else {
						c = LIGHT_COLORS[(id + (int)(t * 2)) % LIGHT_COLOR_COUNT];
						v = 1.0f;
					}
					Color lit = scaleColor(c, v);
					setPixel(f, x, wy + 1, lit);
					Color glow = scaleColor(lit, 0.18f);
					addPixel(f, x - 1, wy + 1, glow);
					addPixel(f, x + 1, wy + 1, glow);
					addPixel(f, x, wy + 2, glow);
				}
			}
		}

	private:
		static constexpr float SWAG = 24.0f;
		float phases[40];
	};

} // namespace

// Get the scene (created on first use)
Scene &getGarlandScene() {
	static GarlandScene scene;
	return scene;
}