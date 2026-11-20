#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// Three crosses on a hill, in silhouette, while the sun rises behind them: the sky goes from night blue through pink and orange to gold, the stars fade out, then it all goes back to dawn (a 16 second loop)
	class CrossesScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "crosses"; }
		const char *name() const override { return "Three crosses"; }

		// Scatter the stars in the upper sky
		void begin() override {
			for (auto &star : stars) {
				star.x = esp_random() % WIDTH;
				star.y = esp_random() % 7;
				star.speed = frand(1.0f, 3.0f);
				star.phase = frand(0, 6.28f);
			}
		}

		// Draw the sky, the stars, the sun, the hill and the crosses
		void draw(Frame &f, float t, float dt) override {
			const float s = sunrise(fmodf(t, LOOP_SECONDS));
			drawSky(f, s);
			drawStars(f, t, s);
			drawSun(f, t, s);
			drawHill(f, s);
			drawCross(f, 36, 1, 3, 2); // The middle cross, taller than the others
			drawCross(f, 26, 3, 5, 1);
			drawCross(f, 46, 3, 5, 1);
		}

	private:
		static constexpr float LOOP_SECONDS = 16.0f;
		static constexpr float RISE_SECONDS = 7.0f; // Time for the sun to rise
		static constexpr float HOLD_SECONDS = 5.0f; // Time to hold the sunrise
		static constexpr float SUN_X = 36.0f; // Column of the sun, behind the middle cross
		static constexpr float SUN_START_Y = 13.5f; // Row of the sun center below the hill
		static constexpr float SUN_END_Y = 5.5f; // Row of the sun center once risen
		static constexpr float SUN_RADIUS = 3.2f;
		static constexpr int HORIZON = 9; // Row of the top of the hill, where the sky ends
		static constexpr int STAR_COUNT = 26;
		static constexpr Color SILHOUETTE = { 3, 2, 6 }; // The hill and the crosses
		static constexpr Color TOP_KEYS[3] = { { 3, 4, 28 }, { 30, 14, 70 }, { 70, 35, 95 } }; // Top of the sky at night, at dawn and at sunrise
		static constexpr Color HORIZON_KEYS[3] = { { 12, 14, 56 }, { 210, 60, 60 }, { 255, 140, 25 } }; // Horizon at night, at dawn and at sunrise

		// A twinkling star
		struct Star {
			uint8_t x, y;
			float speed, phase;
		};

		Star stars[STAR_COUNT] = {};

		// Get how far the sunrise is (0 is night, 1 is risen): it rises, holds, then falls back to dawn
		static float sunrise(float lt) {
			if (lt < RISE_SECONDS) {
				return smooth(lt / RISE_SECONDS);
			}
			if (lt < RISE_SECONDS + HOLD_SECONDS) {
				return 1.0f;
			}
			return smooth(1.0f - (lt - RISE_SECONDS - HOLD_SECONDS) / (LOOP_SECONDS - RISE_SECONDS - HOLD_SECONDS));
		}

		// Ease a value between 0 and 1 in and out
		static float smooth(float x) {
			x = constrain(x, 0.0f, 1.0f);
			return x * x * (3.0f - 2.0f * x);
		}

		// Get the color of a sky gradient at a point of the sunrise (the three keys are night, dawn and sunrise)
		static Color keyColor(const Color *keys, float s) {
			return s < 0.5f ? blendColor(keys[0], keys[1], s * 2.0f) : blendColor(keys[1], keys[2], s * 2.0f - 1.0f);
		}

		// Fill the sky with its gradient, darker at the top and warmer at the horizon
		static void drawSky(Frame &f, float s) {
			const Color top = keyColor(TOP_KEYS, s), horizon = keyColor(HORIZON_KEYS, s);
			for (int y = 0; y < HEIGHT; y++) {
				const float k = powf(min(1.0f, y / (float)HORIZON), 1.4f);
				const Color c = blendColor(top, horizon, k);
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = c;
				}
			}
		}

		// Draw the stars, which fade out as the sky gets light
		void drawStars(Frame &f, float t, float s) const {
			const float visible = constrain(1.0f - s * 1.8f, 0.0f, 1.0f);
			if (visible <= 0) {
				return;
			}
			for (const auto &star : stars) {
				const float v = twinkle(t, star.speed, star.phase);
				addPixel(f, star.x, star.y, scaleColor({ 180, 180, 255 }, v * v * 0.8f * visible));
			}
		}

		// Draw the glow, the soft rays and the disc of the sun
		static void drawSun(Frame &f, float t, float s) {
			const float sunY = SUN_START_Y + (SUN_END_Y - SUN_START_Y) * s;
			const float rays = smooth((s - 0.4f) / 0.5f);
			for (int y = 0; y < HORIZON + 1; y++) {
				for (int x = 0; x < WIDTH; x++) {
					const float dx = x - SUN_X, dy = y - sunY;
					const float d = sqrtf(dx * dx + dy * dy);
					if (d > 16.0f) {
						continue;
					}

					// Draw the disc, orange at the horizon and pale gold when risen
					if (d <= SUN_RADIUS) {
						f[y][x] = blendColor({ 255, 100, 35 }, { 255, 215, 120 }, s);
						continue;
					}

					// Add the glow around the sun, with rays turning slowly through it
					const float glow = 1.0f - d / 16.0f;
					const float ray = 0.5f + 0.5f * sinf(atan2f(dy, dx) * 9.0f + t * 0.5f);
					addPixel(f, x, y, scaleColor({ 255, 150, 40 }, glow * glow * (0.45f + 0.3f * rays * ray) * (0.3f + 0.7f * s)));
				}
			}
		}

		// Get the row where the hill starts in a column: highest in the middle, falling off to the sides
		static int hillTop(int x) {
			const float dx = fabsf(x - SUN_X) / 36.0f;
			return (int)lroundf(HORIZON + 3.0f * powf(dx, 1.7f));
		}

		// Draw the hill in silhouette, with a thin rim of light on top as the sun rises
		static void drawHill(Frame &f, float s) {
			for (int x = 0; x < WIDTH; x++) {
				const int top = hillTop(x);
				for (int y = top; y < HEIGHT; y++) {
					setPixel(f, x, y, y == top ? blendColor(SILHOUETTE, { 120, 50, 20 }, s * 0.7f) : SILHOUETTE);
				}
			}
		}

		// Draw a cross in silhouette, standing on the hill: topY is the top of its post, armY the row of its arms
		static void drawCross(Frame &f, int x, int topY, int armY, int armHalf) {
			for (int y = topY; y < hillTop(x); y++) {
				setPixel(f, x, y, SILHOUETTE);
			}
			for (int dx = -armHalf; dx <= armHalf; dx++) {
				setPixel(f, x + dx, armY, SILHOUETTE);
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getCrossesScene() {
	static CrossesScene scene;
	return scene;
}
