#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Two strings of fairy lights hanging in swags, cycling through light effects
	class GarlandScene : public Scene {
	public:
		// Get the name of the scene
		const char *name() const override { return "Fairy lights"; }

		// Give each bulb a random twinkle phase
		void begin() override {
			for (auto &p : phases) {
				p = frand(0, 6.28f);
			}
		}

		// Draw two strings of lights hanging in swags, with the current light effect
		void draw(Frame &f, float t, float dt) override {
			// Clear the frame
			fillFrame(f, { 0, 0, 0 });

			// Switch the light effect every 6 seconds (twinkle, chase, color cycle)
			int mode = ((int)(t / 6.0f)) % 3;

			// Draw the two strings, the second one lower and shifted sideways
			for (int g = 0; g < 2; g++) {
				// Place the string (top row of its swags and horizontal offset)
				int base = g == 0 ? 0 : 6;
				float offset = g == 0 ? 0 : 12;
				int bulb = 0;

				// Go along the string column by column
				for (int x = 0; x < WIDTH; x++) {
					// Get the row of the wire, sagging up to 3 rows between the two ends of a swag
					float u = fmodf(x + offset, SWAG) - SWAG / 2;
					int wy = base + (int)lroundf(3.0f * (1.0f - (u * u) / (SWAG * SWAG / 4)));

					// Draw the dark green wire
					setPixel(f, x, wy, { 0, 35, 5 });

					// Skip the columns without a bulb (there's one every 4 columns)
					if (((int)(x + offset)) % 4 != 2) {
						continue;
					}

					// Get the color of the bulb from its id
					int id = g * 20 + bulb++;
					Color c = LIGHT_COLORS[id % LIGHT_COLOR_COUNT];

					// Get the brightness of the bulb for the current effect
					float v;
					if (mode == 0) {
						// Twinkle: each bulb pulses on its own phase
						v = 0.15f + 0.85f * powf(twinkle(t, 2.0f, phases[id % 40]), 2);
					} else if (mode == 1) {
						// Chase: one bulb in four is lit and the pattern moves
						v = ((id + (int)(t * 6)) % 4 == 0) ? 1.0f : 0.15f;
					} else {
						// Color cycle: all the bulbs are lit and their colors shift
						c = LIGHT_COLORS[(id + (int)(t * 2)) % LIGHT_COLOR_COUNT];
						v = 1.0f;
					}

					// Draw the bulb under the wire with a soft glow around it
					Color lit = scaleColor(c, v);
					setPixel(f, x, wy + 1, lit);
					Color glow = scaleColor(lit, 0.18f); // Glow at 18% of the bulb brightness
					addPixel(f, x - 1, wy + 1, glow);
					addPixel(f, x + 1, wy + 1, glow);
					addPixel(f, x, wy + 2, glow);
				}
			}
		}

	private:
		static constexpr float SWAG = 24.0f; // Width in columns of one swag of the wire
		float phases[40]; // Twinkle phase of each bulb
	};

} // namespace

// Get the scene (created on first use)
Scene &getGarlandScene() {
	static GarlandScene scene;
	return scene;
}