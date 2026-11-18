#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// A row of Christmas trees with twinkling ornaments and a star on top
	class TreesScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "trees"; }
		const char *name() const override { return "Christmas trees"; }

		// Start the snow and scatter random ornaments on the trees
		void begin() override {
			// Start the snowfall with 16 flakes
			snow.begin(16);

			// Drop the previous ornaments
			ornamentCount = 0;

			// Give each tree random ornaments
			for (const auto &tree : TREES) {
				// Go through the rows below the tip, down to row 10 (above the trunk)
				for (int y = tree.top + 1; y <= 10; y++) {
					int half = halfWidth(tree, y);
					for (int x = tree.cx - half; x <= tree.cx + half; x++) {
						// Add an ornament on 30% of the pixels (until the pool is full)
						if (ornamentCount < MAX_ORNAMENTS && (esp_random() % 100) < 30) {
							ornaments[ornamentCount++] = { (int8_t)x, (int8_t)y, LIGHT_COLORS[esp_random() % LIGHT_COLOR_COUNT],
								frand(1.5f, 5.0f), frand(0, 6.28f) };
						}
					}
				}
			}
		}

		// Draw the trees with their stars and ornaments, and the snow falling in front
		void draw(Frame &f, float t, float dt) override {
			// Fill the night sky
			fillFrame(f, { 0, 0, 20 });
			for (int x = 0; x < WIDTH; x++) {
				// Draw the snowy ground on the bottom row
				f[HEIGHT - 1][x] = { 120, 125, 140 };
			}

			// Draw each tree
			for (const auto &tree : TREES) {
				// Draw the foliage row by row
				for (int y = tree.top; y <= 10; y++) {
					int half = halfWidth(tree, y);
					for (int x = tree.cx - half; x <= tree.cx + half; x++) {
						// Vary the green from pixel to pixel for a leafy look
						uint8_t shade = 90 + ((x * 7 + y * 5) % 3) * 30;
						setPixel(f, x, y, { 0, shade, 20 });
					}
				}

				// Draw the trunk
				setPixel(f, tree.cx, HEIGHT - 1, { 100, 45, 10 });

				// Make the star on the tip twinkle
				float s = 0.6f + 0.4f * twinkle(t, 3.0f, tree.cx);
				setPixel(f, tree.cx, tree.top - 1, scaleColor({ 255, 220, 60 }, s));

				// Add a faint halo above the star when there's room
				if (tree.top >= 2) {
					addPixel(f, tree.cx, tree.top - 2, scaleColor({ 255, 220, 60 }, s * 0.25f));
				}
			}

			// Draw the twinkling ornaments
			for (int i = 0; i < ornamentCount; i++) {
				const auto &o = ornaments[i];
				setPixel(f, o.x, o.y, scaleColor(o.color, 0.3f + 0.7f * twinkle(t, o.speed, o.phase)));
			}

			// Move the flakes (there's no ground to pile up on)
			snow.update(t, dt, nullptr);

			// Draw the flakes in front of the trees
			snow.draw(f, t);
		}

	private:
		// A tree made of rows that widen from its tip down to the trunk
		struct Tree {
			int cx, top; // Center column and row of the tip
		};

		// A light hanging on a tree
		struct Ornament {
			int8_t x, y; // Position
			Color color;
			float speed, phase; // Twinkle speed and phase
		};
		static constexpr Tree TREES[] = { { 6, 5 }, { 18, 1 }, { 31, 4 }, { 44, 1 }, { 57, 5 }, { 67, 2 } }; // Center column and tip row of each tree
		static constexpr int MAX_ORNAMENTS = 120; // Size of the ornament pool
		Ornament ornaments[MAX_ORNAMENTS];
		int ornamentCount = 0; // Number of ornaments in use
		Snowfall snow;

		// Get the half width of a tree at row y (it grows by one column every two rows)
		static int halfWidth(const Tree &tree, int y) { return (y - tree.top + 1) / 2; }
	};

	// Define the trees outside the class (needed before C++17)
	constexpr TreesScene::Tree TreesScene::TREES[];

} // namespace

// Get the scene (created on first use)
Scene &getTreesScene() {
	static TreesScene scene;
	return scene;
}