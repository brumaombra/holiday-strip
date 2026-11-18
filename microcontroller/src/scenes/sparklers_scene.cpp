#include "scene_list.h"

#include <time.h>

#include "calendar/countdown.h"
#include "font/font.h"
#include "scenes/effects.h"

namespace {

	// A sparkler writing the coming year in the air: its trail glows and cools, sparks spit from the tip, then it all fades and starts over (a 10 second loop)
	class SparklersScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "sparklers"; }
		const char *name() const override { return "Sparklers"; }

		// Plan the strokes of the year and clear the trail
		void begin() override {
			planStrokes();
			memset(heat, 0, sizeof(heat));
			for (auto &spark : sparks) {
				spark.life = 0;
			}
			loopTime = 0;
			written = 0;
		}

		// Draw the trail, the sparkler and its sparks
		void draw(Frame &f, float t, float dt) override {
			// Run the 10 second loop, planning the year again at each start
			loopTime += dt;
			if (loopTime >= LOOP_SECONDS) {
				begin();
			}

			// Write the next pixels of the year (the tip moves through them in order)
			const int target = min(pointCount, (int)(loopTime / WRITE_SECONDS * pointCount));
			while (written < target) {
				heat[points[written].y][points[written].x] = 1.0f;
				written++;
			}

			// Cool the trail toward a steady glow, and fade it out at the end of the loop
			const float fadeOut = constrain((LOOP_SECONDS - loopTime) / FADE_SECONDS, 0.0f, 1.0f);
			fillFrame(f, { 0, 0, 3 });
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					float &h = heat[y][x];
					if (h <= 0) {
						continue;
					}
					h = max(GLOW, h - dt * 0.8f);
					setPixel(f, x, y, scaleColor(heatColor(h, t, x), fadeOut));
				}
			}

			// The sparkler: a dark stick held from the bottom up to the tip, which is the last written pixel
			const bool isWriting = written > 0 && written < pointCount;
			if (written > 0 && fadeOut > 0) {
				const Point tip = points[written - 1];
				drawStick(f, tip);
				addPixel(f, tip.x, tip.y, WHITE);

				// Spit sparks while writing, and a few once done
				const int burst = isWriting ? 3 : (esp_random() % 4 == 0 ? 1 : 0);
				for (int i = 0; i < burst; i++) {
					emit(tip.x, tip.y);
				}
			}

			// Move and draw the sparks
			for (auto &spark : sparks) {
				if (spark.life <= 0) {
					continue;
				}
				spark.life -= dt;
				spark.vy += 8.0f * dt; // Gravity
				spark.x += spark.vx * dt;
				spark.y += spark.vy * dt;
				addPixel(f, (int)spark.x, (int)spark.y, scaleColor(blendColor(GOLD, WHITE, spark.life * 2), min(1.0f, spark.life * 3) * fadeOut));
			}
		}

	private:
		static constexpr float LOOP_SECONDS = 10.0f;
		static constexpr float WRITE_SECONDS = 5.0f; // Time to write the whole year
		static constexpr float FADE_SECONDS = 1.2f; // Fade at the end of the loop
		static constexpr float GLOW = 0.25f; // Heat the written pixels cool down to
		static constexpr int MAX_POINTS = 7 * 5 * 8; // Pixels of up to 8 letters
		static constexpr int MAX_SPARKS = 70;

		// A pixel of the year, in writing order
		struct Point {
			uint8_t x, y;
		};

		// A spark from the tip
		struct Spark {
			float x, y, vx, vy, life;
		};

		Point points[MAX_POINTS] = {};
		int pointCount = 0; // Pixels of the year
		int written = 0; // Pixels written so far
		float heat[HEIGHT][WIDTH] = {}; // How hot each pixel of the trail is (1 just written, GLOW once cooled)
		Spark sparks[MAX_SPARKS] = {};
		float loopTime = 0; // Seconds into the loop

		// List the pixels of the coming year in writing order: letter by letter, column by column, going down and up in turn
		void planStrokes() {
			// Read the coming year (a greeting while the clock isn't set)
			struct tm now = {};
			const int year = getLocalTime(&now, 0) ? getUpcomingYear(now) : 0;
			const String text = year > 0 ? String(year) : String("AUGURI");

			// Walk the pixels of each letter, centered on the matrix
			pointCount = 0;
			const int x0 = (WIDTH - textWidth(text) + 1) / 2, y0 = 3;
			for (size_t i = 0; i < text.length(); i++) {
				const Glyph *glyph = findGlyph(text[i]);
				if (glyph == nullptr) {
					continue;
				}
				for (int col = 0; col < 5; col++) {
					for (int step = 0; step < 7; step++) {
						const int row = col % 2 == 0 ? step : 6 - step; // Down, then up
						if ((glyph->cols[col] >> row & 1) && pointCount < MAX_POINTS) {
							points[pointCount++] = { (uint8_t)(x0 + i * GLYPH_ADVANCE + col), (uint8_t)(y0 + row) };
						}
					}
				}
			}
		}

		// Get the color of a trail pixel: white-hot when just written, cooling through gold to a flickering orange glow
		static Color heatColor(float h, float t, int x) {
			if (h > 0.6f) {
				return blendColor(GOLD, WHITE, (h - 0.6f) / 0.4f);
			}
			const float flicker = 0.85f + 0.15f * sinf(t * 9 + x * 1.7f);
			return scaleColor(blendColor({ 255, 80, 0 }, GOLD, (h - GLOW) / (0.6f - GLOW)), (0.5f + h) * flicker);
		}

		// Draw the stick of the sparkler, from the bottom edge up to just below the tip
		static void drawStick(Frame &f, Point tip) {
			for (int y = tip.y + 1; y < HEIGHT; y++) {
				const int x = tip.x + (y - tip.y) / 3; // Leaning a little to the right
				setPixel(f, x, y, { 40, 40, 45 });
			}
		}

		// Shoot a spark from the tip in a random direction
		void emit(int x, int y) {
			for (auto &spark : sparks) {
				if (spark.life > 0) {
					continue;
				}
				const float angle = frand(0, 2 * PI);
				const float speed = frand(4.0f, 12.0f);
				spark = { (float)x, (float)y, cosf(angle) * speed, sinf(angle) * speed * 0.6f, frand(0.15f, 0.45f) };
				return;
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getSparklersScene() {
	static SparklersScene scene;
	return scene;
}