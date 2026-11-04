#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Red and white candy cane stripes sliding diagonally, with sparkles
	class CandyCaneScene : public Scene {
	public:
		const char *name() const override { return "Candy cane"; }

		void draw(Frame &f, float t, float dt) override {
			for (int y = 0; y < HEIGHT; y++) {
				float shade = 0.55f + 0.45f * sinf((y + 0.5f) / HEIGHT * PI);
				for (int x = 0; x < WIDTH; x++) {
					float s = sinf((x + y - t * 8.0f) * 2 * PI / 10.0f);
					float w = constrain(s * 2.5f * 0.5f + 0.5f, 0.0f, 1.0f);
					f[y][x] = scaleColor(blendColor({255, 235, 235}, {230, 0, 0}, w), shade);
				}
			}
			for (int i = 0; i < 3; i++) {
				if (esp_random() % 100 < 15) {
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