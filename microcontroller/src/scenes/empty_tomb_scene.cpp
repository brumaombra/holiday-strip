#include "scene_list.h"

#include "config/language.h"
#include "font/font.h"
#include "scenes/effects.h"

namespace {

	// A rock tomb in a garden at dawn: the stone rolls aside, warm light pours out of the opening, then the news scrolls by (a loop of about 20 seconds)
	class EmptyTombScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "empty-tomb"; }
		const char *name() const override { return "Empty tomb"; }

		// Restart the loop, reading the language again
		void begin() override {
			text = getMatrixLanguage() == ENGLISH ? "HE IS RISEN!" : "CRISTO E' RISORTO!";
			textLength = strlen(text);
			loopTime = 0;
		}

		// Draw the garden, the tomb and its stone, the light, and the scrolling text
		void draw(Frame &f, float t, float dt) override {
			// Run the loop, starting it again once the text has gone
			loopTime += dt;
			const float scrollSeconds = (textLength * GLYPH_ADVANCE + WIDTH) / SCROLL_SPEED;
			const float loopSeconds = TEXT_START + scrollSeconds + 1.2f;
			if (loopTime >= loopSeconds) {
				begin();
			}

			// Get where the stone is and how much light pours out
			const float roll = smooth((loopTime - ROLL_START) / ROLL_SECONDS);
			const float light = smooth((loopTime - LIGHT_START) / LIGHT_SECONDS);

			drawSky(f, smooth(loopTime / 10.0f));
			drawGarden(f);
			drawRock(f, light);
			drawStone(f, STONE_X + (STONE_END_X - STONE_X) * roll);
			drawLight(f, loopTime, light);

			// Fade the scene in and out, and dim it behind the text
			const float dim = 1.0f - 0.7f * smooth((loopTime - TEXT_START + 1.0f) / 1.0f);
			const float fade = min(min(1.0f, loopTime / 0.8f), (loopSeconds - loopTime) / 1.0f);
			for (auto &row : f) {
				for (auto &p : row) {
					p = scaleColor(p, dim * fade);
				}
			}

			// Scroll the text from the right once the light is out
			if (loopTime >= TEXT_START) {
				const float pos = WIDTH - (loopTime - TEXT_START) * SCROLL_SPEED;
				for (int i = 0; i < textLength; i++) {
					const int x = (int)pos + i * GLYPH_ADVANCE;
					if (x < -GLYPH_ADVANCE || x >= WIDTH) {
						continue;
					}
					drawChar(f, text[i], x, 3, [&](int px, int py) { return scaleColor({ 255, 215, 140 }, fade * (0.85f + 0.15f * sinf(px * 0.4f + loopTime * 4.0f))); });
				}
			}
		}

	private:
		static constexpr float ROLL_START = 2.5f; // When the stone starts to roll
		static constexpr float ROLL_SECONDS = 3.5f;
		static constexpr float LIGHT_START = 5.5f; // When the light starts to pour out
		static constexpr float LIGHT_SECONDS = 3.0f;
		static constexpr float TEXT_START = 10.0f; // When the text starts to scroll
		static constexpr float SCROLL_SPEED = 20.0f; // Columns per second
		static constexpr int ROCK_X = 26; // Column of the middle of the tomb and its opening
		static constexpr int GROUND = 10; // First row of the grass
		static constexpr float STONE_X = 26.0f; // Where the stone closes the opening
		static constexpr float STONE_END_X = 51.0f; // Where it ends up, rolled aside
		static constexpr float STONE_Y = 6.5f;
		static constexpr float STONE_RADIUS = 4.4f;
		static constexpr float OPENING_Y = 7.0f; // Row of the middle of the arch of the opening

		const char *text = ""; // Text being scrolled
		int textLength = 0;
		float loopTime = 0; // Seconds into the loop

		// Ease a value between 0 and 1 in and out
		static float smooth(float x) {
			x = constrain(x, 0.0f, 1.0f);
			return x * x * (3.0f - 2.0f * x);
		}

		// Get a stable pseudo-random number from a position, for the texture of the rock
		static uint8_t noise(int x, int y) { return (uint8_t)((x * 73 + y * 151 + x * y * 31) % 13); }

		// Draw the sky of the dawn, getting lighter and warmer toward the horizon
		static void drawSky(Frame &f, float dawn) {
			const Color top = blendColor({ 8, 10, 40 }, { 30, 45, 100 }, dawn);
			const Color horizon = blendColor({ 60, 35, 60 }, { 215, 115, 75 }, dawn);
			for (int y = 0; y < HEIGHT; y++) {
				const Color c = blendColor(top, horizon, min(1.0f, y / 9.0f));
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = c;
				}
			}
		}

		// Draw the grass with a few flowers, and a bush and an olive tree on the sides
		static void drawGarden(Frame &f) {
			// Draw the olive tree and the bush
			for (int y = 8; y < GROUND; y++) {
				setPixel(f, 62, y, { 70, 42, 18 });
			}
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					const float tree = hypotf(x - 62.0f, y - 5.5f), bush = hypotf(x - 3.0f, y - 8.5f);
					if (tree <= 3.6f || bush <= 2.6f) {
						setPixel(f, x, y, noise(x, y) < 5 ? Color { 25, 85, 35 } : Color { 15, 62, 25 });
					}
				}
			}

			// Draw the grass
			for (int y = GROUND; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					setPixel(f, x, y, noise(x, y) < 6 ? Color { 14, 70, 20 } : Color { 9, 50, 14 });
				}
			}
			for (int x = 0; x < WIDTH; x += 3) {
				if (noise(x, 0) > 6) {
					setPixel(f, x, GROUND - 1, { 20, 90, 28 }); // A tuft
				}
			}

			// Draw the flowers
			setPixel(f, 12, 11, { 230, 90, 150 });
			setPixel(f, 17, 10, { 240, 220, 90 });
			setPixel(f, 40, 11, { 240, 220, 90 });
			setPixel(f, 58, 11, { 230, 90, 150 });
			setPixel(f, 69, 10, { 240, 220, 90 });
		}

		// Get the row where the rock mound starts in a column (rounded, highest above the opening)
		static int rockTop(int x) {
			const float dx = (x - ROCK_X) / 20.0f;
			return (int)lroundf(1.0f + 5.0f * dx * dx);
		}

		// Check whether a pixel is inside the arched opening of the tomb
		static bool inOpening(int x, int y) {
			const int dx = x - ROCK_X;
			return y >= (int)OPENING_Y ? (abs(dx) <= 3 && y < GROUND) : (dx * dx + (y - OPENING_Y) * (y - OPENING_Y) <= 11);
		}

		// Draw the rock with its opening, which glows with the light coming from inside
		static void drawRock(Frame &f, float light) {
			for (int x = ROCK_X - 20; x <= ROCK_X + 20; x++) {
				for (int y = rockTop(x); y < GROUND; y++) {
					if (inOpening(x, y)) {
						// Warm up the inside, brightest in the middle
						const float center = 1.0f - hypotf(x - ROCK_X, (y - OPENING_Y) * 0.6f) / 5.0f;
						setPixel(f, x, y, blendColor({ 4, 3, 6 }, { 255, 195, 85 }, light * (0.55f + 0.45f * center)));
						continue;
					}
					const uint8_t n = noise(x, y);
					const float lit = y == rockTop(x) ? 1.4f : 1.0f; // The top edge catches the dawn
					setPixel(f, x, y, scaleColor({ (uint8_t)(62 + n * 2), (uint8_t)(48 + n * 2), (uint8_t)(42 + n * 2) }, lit * (1.0f - 0.25f * (y - 3) / 7.0f)));
				}
			}
		}

		// Draw the round stone at a column, turning as it rolls
		static void drawStone(Frame &f, float sx) {
			const float angle = (sx - STONE_X) / STONE_RADIUS;
			for (int y = 0; y < GROUND + 1; y++) {
				for (int x = (int)(sx - STONE_RADIUS - 1); x <= (int)(sx + STONE_RADIUS + 1); x++) {
					const float dx = x - sx, dy = y - STONE_Y;
					const float d = hypotf(dx, dy);
					if (d > STONE_RADIUS) {
						continue;
					}

					// Shade the stone from its upper left, with a dark crack that turns around the middle
					const float shade = 1.0f - 0.45f * constrain((dx + dy) / (2.0f * STONE_RADIUS) + 0.4f, 0.0f, 1.0f);
					const bool crack = d > 1.0f && fabsf(dx * sinf(angle) - dy * cosf(angle)) < 0.55f;
					setPixel(f, x, y, scaleColor(crack ? Color { 110, 95, 80 } : Color { 215, 200, 170 }, shade));
				}
			}
		}

		// Pour the light out of the opening: a glow around it with soft rays fanning out
		static void drawLight(Frame &f, float lt, float light) {
			if (light <= 0) {
				return;
			}
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					const float dx = x - ROCK_X, dy = y - OPENING_Y;
					const float d = sqrtf(dx * dx + dy * dy);
					if (d > 30.0f) {
						continue;
					}
					const float ray = 0.5f + 0.5f * sinf(atan2f(dy, dx) * 7.0f + lt * 0.8f);
					const float glow = max(0.0f, 1.0f - d / 18.0f);
					addPixel(f, x, y, scaleColor({ 255, 170, 60 }, light * (glow * glow * 0.6f + (1.0f - d / 30.0f) * ray * ray * 0.6f)));
				}
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getEmptyTombScene() {
	static EmptyTombScene scene;
	return scene;
}
