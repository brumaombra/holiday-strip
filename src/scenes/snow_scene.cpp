#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Snow falls and piles up next to a small house with lit windows
	class SnowScene : public Scene {
	public:
		// Get the name of the scene
		const char *name() const override { return "Snowfall"; }

		// Start the snowfall and clear the snow on the ground
		void begin() override {
			// Start the snowfall with 40 flakes
			snow.begin(40);

			// Clear the snow on the ground
			for (auto &g : ground) {
				g = 0;
			}
		}

		// Draw the sky, the house, the falling snow and the snow on the ground
		void draw(Frame &f, float t, float dt) override {
			// Fill the sky with a vertical gradient, lighter near the ground
			for (int y = 0; y < HEIGHT; y++) {
				Color c = blendColor({0, 0, 12}, {0, 6, 35}, y / float(HEIGHT - 1));
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = c;
				}
			}

			// Draw the house
			drawHouse(f, t);

			// Move the flakes, which add to the ground where they land
			snow.update(t, dt, ground);

			// Level the snow between neighboring columns
			settleGround(dt);

			// Draw the snow on the ground
			drawGround(f);

			// Draw the falling flakes on top
			snow.draw(f, t);
		}

	private:
		Snowfall snow;
		float ground[WIDTH]; // Height of the snow on the ground in each column

		// Let the snow slide to the lower neighbor so the piles don't get too steep
		void settleGround(float dt) {
			for (int x = 0; x < WIDTH - 1; x++) {
				// Get the height difference with the next column
				float diff = ground[x] - ground[x + 1];

				// Move some snow to the lower column when the step is higher than one row
				if (fabsf(diff) > 1.0f) {
					float move = diff * 0.5f * dt;
					ground[x] -= move;
					ground[x + 1] += move;
				}
			}
		}

		// Draw the snow on the ground, with a partly lit pixel on top of each pile
		void drawGround(Frame &f) {
			for (int x = 0; x < WIDTH; x++) {
				// Split the height into full pixels and a fraction
				int full = (int)ground[x];
				float frac = ground[x] - full;

				// Fill the full pixels from the bottom
				for (int i = 0; i < full; i++) {
					setPixel(f, x, HEIGHT - 1 - i, SNOW);
				}

				// Blend the top pixel by the fraction (tiny ones are ignored)
				if (frac > 0.05f) {
					int y = HEIGHT - 1 - full;
					if (y >= 0) {
						f[y][x] = blendColor(f[y][x], SNOW, frac);
					}
				}
			}
		}

		// Draw the house with its chimney, roof, walls, door and windows
		static void drawHouse(Frame &f, float t) {
			const int cx = 57; // Center column of the house
			const Color wall = {110, 45, 15};
			const Color roof = {150, 15, 15};
			// Chimney
			for (int y = 2; y <= 4; y++) {
				setPixel(f, 61, y, {120, 40, 30});
			}
			setPixel(f, 61, 1, SNOW);
			// Roof with snow on its top edge
			for (int y = 3; y <= 6; y++) {
				// Widen the roof by two columns on each row
				int half = 1 + (y - 3) * 2;
				int halfAbove = y == 3 ? -1 : 1 + (y - 4) * 2;
				for (int x = cx - half; x <= cx + half; x++) {
					// Mark the pixels wider than the row above as the snowy edge
					bool top = abs(x - cx) > halfAbove;
					setPixel(f, x, y, top ? SNOW : roof);
				}
			}

			// Draw the walls
			for (int y = 7; y < HEIGHT; y++) {
				for (int x = cx - 6; x <= cx + 6; x++) {
					setPixel(f, x, y, wall);
				}
			}
			// Door and warm flickering windows
			for (int y = 9; y < HEIGHT; y++) {
				setPixel(f, cx, y, {50, 20, 5});
			}

			// Make the light of the windows flicker
			float flicker = 0.8f + 0.2f * sinf(t * 7.0f) * sinf(t * 2.3f);
			Color light = scaleColor({255, 170, 50}, flicker);

			// Draw the two windows, two pixels wide and tall
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