#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Wrapped presents fall from the sky and stack up into a pile
	class GiftsScene : public Scene {
	public:
		// Get the name of the scene
		const char *name() const override { return "Gifts"; }

		// Empty the pile and drop the gift that was falling
		void begin() override {
			clearPile();
			falling.active = false;
			sinking = 0;
		}

		// Draw the sky, the pile and the falling gift (a full pile sinks away)
		void draw(Frame &f, float t, float dt) override {
			// Draw the night sky with its stars
			fillFrame(f, {0, 0, 12});
			stars.draw(f, t);

			// Count down the sinking of a full pile and empty it when done
			if (sinking > 0) {
				sinking -= dt;
				if (sinking <= 0) {
					clearPile();
				}
			}

			// Get how far the pile has sunk (0 when it isn't sinking)
			int offset = sinking > 0 ? (int)((1.0f - sinking / SINK_SECONDS) * HEIGHT) : 0;

			// Draw the pile shifted down by the sinking offset
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					if (pileUsed[y][x] && y + offset < HEIGHT) {
						f[y + offset][x] = pile[y][x];
					}
				}
			}

			// Stop here while the pile sinks, so no new gift falls
			if (sinking > 0) {
				return;
			}

			// Start a new gift above the pile when none is falling
			if (!falling.active) {
				spawn();
			}

			// Accelerate the gift with gravity (25 rows per second squared)
			falling.speed += 25.0f * dt;
			falling.y += falling.speed * dt;

			// Find the row where the gift lands on the pile
			int landY = landingTop(falling.x, falling.w);

			// Stack the gift when it lands, otherwise draw it falling
			if (falling.y + falling.h >= landY) {
				// Snap the gift on top of the pile and add it to the pile
				falling.y = landY - falling.h;
				paintGift(falling, (int)falling.y, true, f);
				falling.active = false;

				// Sink the pile when the new gift reaches the top rows
				if (landY - falling.h < 4) {
					sinking = SINK_SECONDS;
				}
			} else {
				// Draw the gift in the air
				paintGift(falling, (int)falling.y, false, f);
			}
		}

		// Place the stars once
		GiftsScene() { stars.begin(); }

	private:
		// A wrapped gift
		struct Gift {
			int x, w, h;    // Left column, width and height
			float y, speed; // Top row and fall speed (rows per second)
			Color box, ribbon;
			bool active; // Whether the gift is falling
		};
		static constexpr float SINK_SECONDS = 1.2f; // Seconds a full pile takes to sink away
		Color pile[HEIGHT][WIDTH];             // Colors of the landed gifts
		bool pileUsed[HEIGHT][WIDTH];          // Which pixels of the pile are filled
		int top[WIDTH];                        // Top row of the boxes in each column (HEIGHT when empty)
		Gift falling;                          // The gift that is falling now
		float sinking = 0;                          // Seconds left before the pile has sunk (0 when it isn't sinking)
		Stars stars;

		// Remove all the gifts from the pile
		void clearPile() {
			// Mark every pixel of the pile as empty
			memset(pileUsed, 0, sizeof(pileUsed));

			// Reset the top of every column to the floor
			for (auto &t : top) {
				t = HEIGHT;
			}
		}

		// Get the top row of the highest column under a gift of width w at column x
		int landingTop(int x, int w) const {
			// Start from the floor
			int result = HEIGHT;

			// Keep the highest top of the columns under the gift
			for (int i = x; i < x + w; i++) {
				result = min(result, top[i]);
			}

			// Return the top row of the highest column
			return result;
		}

		// Create a gift with a random size, colors and position above the screen
		void spawn() {
			// Define the colors of the boxes and the ribbons
			static const Color boxes[] = {{210, 0, 0}, {0, 150, 30}, {20, 60, 230}, {150, 0, 160}, {230, 120, 0}};
			static const Color ribbons[] = {GOLD, WHITE, {255, 40, 40}};

			// Pick a random size (5 to 7 wide, 3 to 4 tall)
			falling.w = 5 + esp_random() % 3;
			falling.h = 3 + esp_random() % 2;
			// Prefer the lowest spots so the pile grows evenly
			int bestX = 0, bestTop = -1;
			for (int tries = 0; tries < 6; tries++) { // Try 6 random columns
				int x = esp_random() % (WIDTH - falling.w + 1);
				int t = landingTop(x, falling.w);
				if (t > bestTop) {
					bestTop = t, bestX = x;
				}
			}

			// Place the gift above the screen and start it falling slowly
			falling.x = bestX;
			falling.y = -falling.h - 2;
			falling.speed = 2.0f; // Rows per second

			// Pick random colors and start the fall
			falling.box = boxes[esp_random() % 5];
			falling.ribbon = ribbons[esp_random() % 3];
			falling.active = true;
		}

		// Draw a gift (a box with a ribbon cross and a bow on top), or add it to the pile when it has landed
		void paintGift(const Gift &g, int y, bool land, Frame &f) {
			// Get the middle column, where the vertical ribbon goes
			int cx = g.x + g.w / 2;
			// Bows don't count as support, so gifts sit flat on the boxes below
			auto put = [&](int x, int py, Color c, bool solid = true) {
				if (land) {
					// Store the pixel in the pile (skipping the ones outside the matrix)
					if (x >= 0 && x < WIDTH && py >= 0 && py < HEIGHT) {
						pile[py][x] = c;
						pileUsed[py][x] = true;

						// Raise the top of the column when the pixel can support a gift
						if (solid) {
							top[x] = min(top[x], py);
						}
					}
				} else {
					// Draw the pixel directly while the gift is falling
					setPixel(f, x, py, c);
				}
			};

			// Draw the box with its ribbon cross
			for (int r = 0; r < g.h; r++) {
				for (int c = 0; c < g.w; c++) {
					int x = g.x + c;

					// Use the ribbon color on the middle column and the middle row
					bool ribbon = x == cx || r == g.h / 2;
					put(x, y + r, ribbon ? g.ribbon : g.box);
				}
			}

			// Draw the two loops of the bow above the box
			put(cx - 1, y - 1, g.ribbon, false);
			put(cx + 1, y - 1, g.ribbon, false);
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getGiftsScene() {
	static GiftsScene scene;
	return scene;
}