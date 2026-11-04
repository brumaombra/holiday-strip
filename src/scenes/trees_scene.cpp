#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// A row of Christmas trees with twinkling ornaments and a star on top
	class TreesScene : public Scene {
	public:
		const char *name() const override { return "Christmas trees"; }

		void begin() override {
			snow.begin(16);
			ornamentCount = 0;
			for (const auto &tree : TREES) {
				for (int y = tree.top + 1; y <= 10; y++) {
					int half = halfWidth(tree, y);
					for (int x = tree.cx - half; x <= tree.cx + half; x++) {
						if (ornamentCount < MAX_ORNAMENTS && (esp_random() % 100) < 30) {
							ornaments[ornamentCount++] = {(int8_t)x, (int8_t)y, LIGHT_COLORS[esp_random() % LIGHT_COLOR_COUNT],
							                              frand(1.5f, 5.0f), frand(0, 6.28f)};
						}
					}
				}
			}
		}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, {0, 0, 20});
			for (int x = 0; x < WIDTH; x++) {
				f[HEIGHT - 1][x] = {120, 125, 140};
			}

			for (const auto &tree : TREES) {
				for (int y = tree.top; y <= 10; y++) {
					int half = halfWidth(tree, y);
					for (int x = tree.cx - half; x <= tree.cx + half; x++) {
						uint8_t shade = 90 + ((x * 7 + y * 5) % 3) * 30;
						setPixel(f, x, y, {0, shade, 20});
					}
				}
				setPixel(f, tree.cx, HEIGHT - 1, {100, 45, 10});
				float s = 0.6f + 0.4f * twinkle(t, 3.0f, tree.cx);
				setPixel(f, tree.cx, tree.top - 1, scaleColor({255, 220, 60}, s));
				if (tree.top >= 2) {
					addPixel(f, tree.cx, tree.top - 2, scaleColor({255, 220, 60}, s * 0.25f));
				}
			}

			for (int i = 0; i < ornamentCount; i++) {
				const auto &o = ornaments[i];
				setPixel(f, o.x, o.y, scaleColor(o.color, 0.3f + 0.7f * twinkle(t, o.speed, o.phase)));
			}

			snow.update(t, dt, nullptr);
			snow.draw(f, t);
		}

	private:
		struct Tree {
			int cx, top;
		};
		struct Ornament {
			int8_t x, y;
			Color color;
			float speed, phase;
		};
		static constexpr Tree TREES[] = {{6, 5}, {18, 1}, {31, 4}, {44, 1}, {57, 5}, {67, 2}};
		static constexpr int MAX_ORNAMENTS = 120;
		Ornament ornaments[MAX_ORNAMENTS];
		int ornamentCount = 0;
		Snowfall snow;

		static int halfWidth(const Tree &tree, int y) { return (y - tree.top + 1) / 2; }
	};
	constexpr TreesScene::Tree TreesScene::TREES[];

} // namespace

// Get the scene (created on first use)
Scene &getTreesScene() {
	static TreesScene scene;
	return scene;
}