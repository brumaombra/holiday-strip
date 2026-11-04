#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Wrapped presents fall from the sky and stack up into a pile
	class GiftsScene : public Scene {
	public:
		const char *name() const override { return "Gifts"; }

		void begin() override {
			clearPile();
			falling.active = false;
			sinking = 0;
		}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, {0, 0, 12});
			stars.draw(f, t);

			if (sinking > 0) {
				sinking -= dt;
				if (sinking <= 0) {
					clearPile();
				}
			}
			int offset = sinking > 0 ? (int)((1.0f - sinking / SINK_SECONDS) * HEIGHT) : 0;
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					if (pileUsed[y][x] && y + offset < HEIGHT) {
						f[y + offset][x] = pile[y][x];
					}
				}
			}

			if (sinking > 0) {
				return;
			}
			if (!falling.active) {
				spawn();
			}
			falling.speed += 25.0f * dt;
			falling.y += falling.speed * dt;
			int landY = landingTop(falling.x, falling.w);
			if (falling.y + falling.h >= landY) {
				falling.y = landY - falling.h;
				paintGift(falling, (int)falling.y, true, f);
				falling.active = false;
				if (landY - falling.h < 4) {
					sinking = SINK_SECONDS;
				}
			} else {
				paintGift(falling, (int)falling.y, false, f);
			}
		}

		GiftsScene() { stars.begin(); }

	private:
		struct Gift {
			int x, w, h;
			float y, speed;
			Color box, ribbon;
			bool active;
		};
		static constexpr float SINK_SECONDS = 1.2f;
		Color pile[HEIGHT][WIDTH];
		bool pileUsed[HEIGHT][WIDTH];
		int top[WIDTH];
		Gift falling;
		float sinking = 0;
		Stars stars;

		void clearPile() {
			memset(pileUsed, 0, sizeof(pileUsed));
			for (auto &t : top) {
				t = HEIGHT;
			}
		}

		int landingTop(int x, int w) const {
			int result = HEIGHT;
			for (int i = x; i < x + w; i++) {
				result = min(result, top[i]);
			}
			return result;
		}

		void spawn() {
			static const Color boxes[] = {{210, 0, 0}, {0, 150, 30}, {20, 60, 230}, {150, 0, 160}, {230, 120, 0}};
			static const Color ribbons[] = {GOLD, WHITE, {255, 40, 40}};
			falling.w = 5 + esp_random() % 3;
			falling.h = 3 + esp_random() % 2;
			// Prefer the lowest spots so the pile grows evenly
			int bestX = 0, bestTop = -1;
			for (int tries = 0; tries < 6; tries++) {
				int x = esp_random() % (WIDTH - falling.w + 1);
				int t = landingTop(x, falling.w);
				if (t > bestTop) {
					bestTop = t, bestX = x;
				}
			}
			falling.x = bestX;
			falling.y = -falling.h - 2;
			falling.speed = 2.0f;
			falling.box = boxes[esp_random() % 5];
			falling.ribbon = ribbons[esp_random() % 3];
			falling.active = true;
		}

		// Box with a ribbon cross and a bow on top; landed gifts are added to the pile
		void paintGift(const Gift &g, int y, bool land, Frame &f) {
			int cx = g.x + g.w / 2;
			// Bows don't count as support, so gifts sit flat on the boxes below
			auto put = [&](int x, int py, Color c, bool solid = true) {
				if (land) {
					if (x >= 0 && x < WIDTH && py >= 0 && py < HEIGHT) {
						pile[py][x] = c;
						pileUsed[py][x] = true;
						if (solid) {
							top[x] = min(top[x], py);
						}
					}
				} else {
					setPixel(f, x, py, c);
				}
			};
			for (int r = 0; r < g.h; r++) {
				for (int c = 0; c < g.w; c++) {
					int x = g.x + c;
					bool ribbon = x == cx || r == g.h / 2;
					put(x, y + r, ribbon ? g.ribbon : g.box);
				}
			}
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