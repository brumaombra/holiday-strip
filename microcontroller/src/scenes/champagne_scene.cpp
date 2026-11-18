#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// A champagne bottle that shakes and pops its cork, foam spraying out, then two glasses filling with bubbles and clinking (a 12 second loop)
	class ChampagneScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "champagne"; }
		const char *name() const override { return "Champagne"; }

		// Start the loop from the shaking bottle
		void begin() override {
			for (auto &drop : drops) {
				drop.life = 0;
			}
			for (auto &bubble : bubbles) {
				bubble.y = -1;
			}
			loopTime = 0;
		}

		// Draw the step of the loop the scene is at
		void draw(Frame &f, float t, float dt) override {
			// Run the 12 second loop
			const float previous = loopTime;
			loopTime = fmodf(loopTime + dt, LOOP_SECONDS);
			if (loopTime < previous) {
				begin();
			}

			// Start from a dark warm night, with the stars
			fillFrame(f, { 3, 1, 6 });
			stars.draw(f, t);

			// The bottle: shaking, then still once the cork is gone
			const int shake = loopTime < POP_SECONDS ? (int)lroundf(sinf(loopTime * 40.0f) * min(1.0f, loopTime)) : 0;
			drawBottle(f, BOTTLE_X + shake, loopTime < POP_SECONDS);

			// The pop: a flash at the neck, the cork flying off and the foam spraying out
			if (loopTime >= POP_SECONDS && previous < POP_SECONDS) {
				spray();
			}
			if (loopTime >= POP_SECONDS && loopTime < POP_SECONDS + 0.15f) {
				for (int dy = -1; dy <= 1; dy++) {
					for (int dx = -1; dx <= 1; dx++) {
						addPixel(f, NECK_X + dx, 0 + dy, WHITE);
					}
				}
			}
			drawCork(f, loopTime - POP_SECONDS);
			updateDrops(f, dt);

			// The glasses: filling, then sliding together and clinking
			drawGlasses(f, t, dt);
		}

		// Place the stars once
		ChampagneScene() { stars.begin(); }

	private:
		static constexpr float LOOP_SECONDS = 12.0f;
		static constexpr float POP_SECONDS = 2.0f; // When the cork pops
		static constexpr float FILL_START = 3.0f, FILL_END = 7.0f; // When the glasses fill
		static constexpr float CLINK_SECONDS = 9.0f; // When the glasses touch
		static constexpr int BOTTLE_X = 3; // Left column of the bottle
		static constexpr int NECK_X = BOTTLE_X + 2; // Column of the bottle neck
		static constexpr int MAX_DROPS = 60, MAX_BUBBLES = 12;
		static constexpr Color BOTTLE_GREEN = { 10, 90, 30 };
		static constexpr Color FOIL = { 230, 170, 30 };
		static constexpr Color CHAMPAGNE = { 255, 190, 60 };
		static constexpr Color GLASS = { 70, 90, 110 };

		// A drop of foam
		struct Drop {
			float x, y, vx, vy, life;
		};

		// A bubble rising in a glass
		struct Bubble {
			float y, speed;
			int glass, column;
		};

		Stars stars;
		Drop drops[MAX_DROPS] = {};
		Bubble bubbles[MAX_BUBBLES] = {};
		float loopTime = 0; // Seconds into the 12 second loop

		// Draw the bottle: the body, the label, the gold foil neck and, before the pop, the cork
		static void drawBottle(Frame &f, int x, bool hasCork) {
			// The body, 5 columns wide from row 5 to the bottom, with a gold label
			for (int y = 5; y < HEIGHT; y++) {
				for (int dx = 0; dx < 5; dx++) {
					setPixel(f, x + dx, y, (y >= 7 && y <= 9) ? FOIL : BOTTLE_GREEN);
				}
			}

			// The shoulders and the neck, wrapped in foil
			for (int dx = 1; dx < 4; dx++) {
				setPixel(f, x + dx, 4, BOTTLE_GREEN);
			}
			for (int y = 1; y < 4; y++) {
				setPixel(f, x + 2, y, FOIL);
			}

			// The cork on top
			if (hasCork) {
				setPixel(f, x + 2, 0, { 200, 150, 90 });
			}
		}

		// Draw the cork flying off to the right in an arc (seconds since the pop; nothing before it)
		static void drawCork(Frame &f, float time) {
			if (time < 0 || time > 1.5f) {
				return;
			}
			const float x = NECK_X + time * 40.0f;
			const float y = -time * 6.0f + time * time * 9.0f; // Up, then down
			setPixel(f, (int)x, (int)y, { 200, 150, 90 });
		}

		// Spray the foam out of the neck, up and to the right
		void spray() {
			for (auto &drop : drops) {
				const float angle = frand(-1.45f, -0.4f); // Radians, up and right
				const float speed = frand(6.0f, 20.0f);
				drop = { (float)NECK_X, 0.5f, cosf(angle) * speed, sinf(angle) * speed * 0.5f, frand(0.8f, 2.0f) };
			}
		}

		// Move and draw the foam drops, falling back down
		void updateDrops(Frame &f, float dt) {
			for (auto &drop : drops) {
				if (drop.life <= 0) {
					continue;
				}
				drop.life -= dt;
				drop.vy += 9.0f * dt; // Gravity
				drop.vx *= 1.0f - 0.8f * dt; // Drag
				drop.x += drop.vx * dt;
				drop.y += drop.vy * dt;
				addPixel(f, (int)drop.x, (int)drop.y, scaleColor({ 255, 245, 210 }, min(1.0f, drop.life)));
			}
		}

		// Draw the two glasses: filling, bubbling, sliding together and clinking with a sparkle
		void drawGlasses(Frame &f, float t, float dt) {
			// Slide the glasses together before the clink, and apart after it
			const float together = constrain(1.0f - fabsf(loopTime - CLINK_SECONDS) / 1.2f, 0.0f, 1.0f);
			const int gap = (int)lroundf(10 - together * 4);
			const int leftX = 52 - gap / 2 - 2, rightX = 52 + gap / 2 + 2; // Centers of the two glasses
			const float fill = constrain((loopTime - FILL_START) / (FILL_END - FILL_START), 0.0f, 1.0f);

			// Each glass: a flute bowl from row 2 to 7, a stem and a base, the champagne filling the bowl from the bottom
			for (int glass = 0; glass < 2; glass++) {
				const int cx = glass == 0 ? leftX : rightX;
				const int level = 7 - (int)lroundf(fill * 5); // Highest row of champagne
				for (int y = 2; y <= 7; y++) {
					setPixel(f, cx - 2, y, GLASS);
					setPixel(f, cx + 2, y, GLASS);
					for (int dx = -1; dx <= 1; dx++) {
						if (fill > 0 && y >= level) {
							setPixel(f, cx + dx, y, scaleColor(CHAMPAGNE, 0.55f + 0.1f * sinf(t * 3 + dx + y)));
						}
					}
				}
				for (int dx = -1; dx <= 1; dx++) {
					setPixel(f, cx + dx, 8, GLASS);
					setPixel(f, cx + dx, 11, GLASS);
				}
				setPixel(f, cx, 9, GLASS);
				setPixel(f, cx, 10, GLASS);

				// Rising bubbles in the champagne
				if (fill > 0.2f) {
					updateBubbles(f, glass, cx, level, dt);
				}
			}

			// The clink: a white star where the glasses touch
			const float clink = 1.0f - fabsf(loopTime - CLINK_SECONDS) / 0.4f;
			if (clink > 0) {
				const int x = (leftX + rightX) / 2, y = 1;
				const Color sparkle = scaleColor(WHITE, clink);
				addPixel(f, x, y, sparkle);
				for (int d = 1; d <= 2; d++) {
					addPixel(f, x - d, y, scaleColor(sparkle, 0.6f / d));
					addPixel(f, x + d, y, scaleColor(sparkle, 0.6f / d));
					addPixel(f, x, y + d, scaleColor(sparkle, 0.6f / d));
				}
			}
		}

		// Move the bubbles of one glass up through the champagne, and start new ones at the bottom
		void updateBubbles(Frame &f, int glass, int cx, int level, float dt) {
			for (auto &bubble : bubbles) {
				// Start a bubble in a free slot now and then
				if (bubble.y < 0) {
					if (esp_random() % 40 == 0) {
						bubble = { 7.0f, frand(3.0f, 6.0f), glass, (int)(esp_random() % 3) - 1 };
					}
					continue;
				}
				if (bubble.glass != glass) {
					continue;
				}

				// Rise until the surface
				bubble.y -= bubble.speed * dt;
				if (bubble.y < level) {
					bubble.y = -1;
					continue;
				}
				addPixel(f, cx + bubble.column, (int)bubble.y, { 120, 110, 80 });
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getChampagneScene() {
	static ChampagneScene scene;
	return scene;
}