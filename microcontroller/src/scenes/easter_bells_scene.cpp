#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// Three golden bells swinging one after the other under a red ribbon, with ringing marks and sparkles at the end of each swing (a 13 second loop)
	class EasterBellsScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "easter-bells"; }
		const char *name() const override { return "Easter bells"; }

		// Clear the sparkles and the swing state
		void begin() override {
			for (auto &sparkle : sparkles) {
				sparkle.life = 0;
			}
			for (auto &c : lastCos) {
				c = 1.0f;
			}
			for (auto &s : stars) {
				s.x = esp_random() % WIDTH;
				s.y = 2 + esp_random() % 10;
				s.speed = frand(1.0f, 3.0f);
				s.phase = frand(0, 6.28f);
			}
		}

		// Draw the night, the ribbon and the three bells with their ringing marks
		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, { 6, 4, 24 });
			drawStars(f, t);
			drawRibbon(f);

			for (int i = 0; i < BELL_COUNT; i++) {
				// Swing the bell back and forth (the bells are a bit out of phase)
				const float phase = t * 2.0f * PI / SWING_SECONDS + i * PHASE_STEP;
				const float swing = sinf(phase);
				const float x = BELL_X0 + i * BELL_SPACING;

				// Spit sparkles when the bell reaches the end of its swing
				const float c = cosf(phase);
				if ((c > 0) != (lastCos[i] > 0)) {
					emit(x + swing * 5.0f, 9.0f);
				}
				lastCos[i] = c;

				drawBell(f, x, MAX_ANGLE * swing);
				drawRing(f, x, swing);
			}
			updateSparkles(f, dt);
		}

	private:
		static constexpr float SWING_SECONDS = 2.6f; // Time of a full swing back and forth (five swings in the loop)
		static constexpr float PHASE_STEP = 0.9f; // Delay of each bell behind the previous one, in radians of the swing
		static constexpr float MAX_ANGLE = 0.42f; // Angle at the end of the swing, in radians
		static constexpr int BELL_COUNT = 3;
		static constexpr float BELL_X0 = 16.0f; // Column of the pivot of the first bell
		static constexpr float BELL_SPACING = 20.0f;
		static constexpr float PIVOT_Y = 2.0f; // Row where the bells hang from the ribbon
		static constexpr float CLAPPER = 8.4f; // Distance of the clapper from the pivot
		static constexpr int STAR_COUNT = 14;
		static constexpr int MAX_SPARKLES = 20;

		// A faint star in the background
		struct Star {
			uint8_t x, y;
			float speed, phase;
		};

		// A sparkle thrown by a bell
		struct Sparkle {
			float x, y, vx, vy, life;
		};

		Star stars[STAR_COUNT] = {};
		Sparkle sparkles[MAX_SPARKLES] = {};
		float lastCos[BELL_COUNT] = {}; // Sign of the swing speed of each bell last frame, to catch the ends of the swing

		// Draw the faint stars
		void drawStars(Frame &f, float t) const {
			for (const auto &s : stars) {
				const float v = twinkle(t, s.speed, s.phase);
				addPixel(f, s.x, s.y, scaleColor({ 150, 150, 220 }, v * v * 0.5f));
			}
		}

		// Draw the red ribbon the bells hang from, with a bow at each end
		static void drawRibbon(Frame &f) {
			for (int x = 2; x < WIDTH - 2; x++) {
				setPixel(f, x, 0, { 220, 25, 45 });
				setPixel(f, x, 1, { 150, 10, 30 });
			}
			for (const int bx : { 5, WIDTH - 6 }) {
				for (int dx = -2; dx <= 2; dx++) {
					setPixel(f, bx + dx, 0, { 255, 70, 85 });
					setPixel(f, bx + dx, 1, { 220, 25, 45 });
				}
				setPixel(f, bx, 2, { 150, 10, 30 });
				setPixel(f, bx, 3, { 150, 10, 30 });
			}
		}

		// Get the half width of the bell at a distance v below its pivot (0 where there is no bell)
		static float halfWidth(float v) {
			if (v < 0.6f) {
				return 0.5f; // The ring it hangs from
			}
			if (v < 1.6f) {
				return 1.6f + (v - 0.6f) * 1.2f; // The round top
			}
			if (v < 6.2f) {
				return 2.8f + (v - 1.6f) * 0.28f; // The body, flaring out
			}
			return v < 7.3f ? 4.5f : 0.0f; // The lip
		}

		// Draw a bell swinging by an angle from its pivot, shaded from its left side, with its clapper
		static void drawBell(Frame &f, float px, float angle) {
			const float s = sinf(angle), c = cosf(angle);
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = (int)(px - 9); x <= (int)(px + 9); x++) {
					// Turn the pixel back into the coordinates of the bell
					const float dx = x + 0.5f - (px + 0.5f), dy = y + 0.5f - PIVOT_Y;
					const float u = dx * c + dy * s, v = -dx * s + dy * c;
					const float w = halfWidth(v);
					if (fabsf(u) > w) {
						continue;
					}

					// Shade from bright gold on the left to dark gold on the right, with a dark mouth under the lip
					const float shade = (u / w + 1.0f) / 2.0f;
					Color color = shade < 0.3f ? Color { 255, 225, 100 } : shade < 0.7f ? Color { 240, 165, 20 } : Color { 175, 105, 10 };
					if (v >= 6.2f) {
						color = scaleColor(color, 0.75f);
					}
					setPixel(f, x, y, color);
				}
			}

			// The clapper hangs below the lip, on the line of the bell
			setPixel(f, (int)floorf(px + 0.5f - CLAPPER * s), (int)floorf(PIVOT_Y + CLAPPER * c), { 255, 200, 60 });
		}

		// Draw the ringing marks beside a bell at the end of its swing, on the side it swung to
		static void drawRing(Frame &f, float px, float swing) {
			const float strength = constrain((fabsf(swing) - 0.82f) / 0.18f, 0.0f, 1.0f);
			if (strength <= 0) {
				return;
			}
			const int dir = swing > 0 ? 1 : -1;
			const int x = (int)(px + swing * 4.5f) + dir * 6;
			const Color color = scaleColor({ 255, 230, 140 }, strength);
			for (int dy = -1; dy <= 1; dy++) {
				setPixel(f, x + (dy == 0 ? dir : 0), 6 + dy, color);
			}
			for (int dy = -3; dy <= 3; dy++) {
				setPixel(f, x + dir * (abs(dy) == 3 ? 2 : 3), 6 + dy, scaleColor(color, 0.6f));
			}
		}

		// Throw a few sparkles from a point
		void emit(float x, float y) {
			for (int n = 0; n < 4; n++) {
				for (auto &sparkle : sparkles) {
					if (sparkle.life <= 0) {
						const float angle = frand(0, 2 * PI);
						sparkle = { x, y, cosf(angle) * frand(3.0f, 8.0f), sinf(angle) * frand(2.0f, 6.0f), frand(0.3f, 0.7f) };
						break;
					}
				}
			}
		}

		// Move the sparkles and draw them, fading as they age
		void updateSparkles(Frame &f, float dt) {
			for (auto &sparkle : sparkles) {
				if (sparkle.life <= 0) {
					continue;
				}
				sparkle.life -= dt;
				sparkle.x += sparkle.vx * dt;
				sparkle.y += sparkle.vy * dt;
				addPixel(f, (int)sparkle.x, (int)sparkle.y, scaleColor({ 255, 230, 140 }, min(1.0f, sparkle.life * 3.0f)));
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getEasterBellsScene() {
	static EasterBellsScene scene;
	return scene;
}
