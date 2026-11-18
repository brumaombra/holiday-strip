#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Red and white candy cane stripes sliding diagonally, with sparkles
	class CandyCaneScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "candy-cane"; }
		const char *name() const override { return "Candy cane"; }

		// Draw the sliding diagonal stripes and a few random sparkles
		void draw(Frame &f, float t, float dt) override {
			// Draw the stripes row by row
			for (int y = 0; y < HEIGHT; y++) {
				// Darken the top and bottom rows so the cane looks rounded
				float shade = 0.55f + 0.45f * sinf((y + 0.5f) / HEIGHT * PI);
				for (int x = 0; x < WIDTH; x++) {
					// Get the stripe wave at this pixel (10 pixels per period, sliding 8 pixels per second)
					float s = sinf((x + y - t * 8.0f) * 2 * PI / 10.0f);

					// Sharpen the wave into stripes with crisp edges
					float w = constrain(s * 2.5f * 0.5f + 0.5f, 0.0f, 1.0f);

					// Blend from white to red along the stripe and apply the row shade
					f[y][x] = scaleColor(blendColor({ 255, 235, 235 }, { 230, 0, 0 }, w), shade);
				}
			}

			// Add a few white sparkles at random places
			for (int i = 0; i < 3; i++) { // Three tries per frame
				if (esp_random() % 100 < 15) { // 15% chance per try
					addPixel(f, esp_random() % WIDTH, esp_random() % HEIGHT, WHITE);
				}
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getCandyCaneScene() {
	static CandyCaneScene scene;
	return scene;
}