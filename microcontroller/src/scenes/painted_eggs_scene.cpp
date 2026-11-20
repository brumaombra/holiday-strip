#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// A row of eggs painted one after another, a brush adding the base color then stripes, zigzags and dots; they all shine, fade, and start again with new colors (a 14 second loop)
	class PaintedEggsScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "painted-eggs"; }
		const char *name() const override { return "Painted eggs"; }

		// Restart the painting with new colors and patterns
		void begin() override {
			loopTime = 0;
			const uint8_t hue = esp_random() % 256;
			const int first = esp_random() % PATTERN_COUNT;
			for (int i = 0; i < EGG_COUNT; i++) {
				base[i] = hsvColor(hue + i * 51, 150, 235);
				accent[i] = i % 2 ? Color{ 255, 240, 200 } : hsvColor(hue + i * 51 + 128, 210, 255);
				pattern[i] = (first + i) % PATTERN_COUNT;
			}
		}

		// Draw the eggs at their stage of painting, then the brush, then the shine
		void draw(Frame &f, float t, float dt) override {
			// Run the loop, starting again with new colors
			loopTime += dt;
			if (loopTime >= LOOP_SECONDS) {
				begin();
			}

			// Draw the dark background with a strip of grass
			fillFrame(f, { 6, 4, 16 });
			for (int x = 0; x < WIDTH; x++) {
				setPixel(f, x, 11, { 10, 60, 24 });
			}

			// Fade in at the start and out at the end
			const float fade = min(constrain(loopTime / 0.5f, 0.0f, 1.0f), constrain((LOOP_SECONDS - loopTime) / FADE_SECONDS, 0.0f, 1.0f));

			// Get the position of the shine that sweeps over the finished eggs
			const float shine = (loopTime - SHINE_START) / SHINE_SECONDS * (WIDTH + 16) - 8;
			const bool isShining = loopTime >= SHINE_START;

			// Draw the eggs
			for (int i = 0; i < EGG_COUNT; i++) {
				drawEgg(f, i, t, fade, shine, isShining);
			}

			// Draw the brush on the egg being painted
			for (int i = 0; i < EGG_COUNT; i++) {
				const float local = loopTime - i * SLOT_SECONDS;
				if (local >= 0 && local < BASE_SECONDS + PATTERN_SECONDS) {
					drawBrush(f, i, local);
				}
			}
		}

	private:
		static constexpr float LOOP_SECONDS = 14.0f;
		static constexpr float FADE_SECONDS = 1.0f; // Fade at the end of the loop
		static constexpr float SLOT_SECONDS = 2.0f; // Time between the starts of two eggs
		static constexpr float BASE_SECONDS = 0.7f; // Time to paint the base color
		static constexpr float PATTERN_SECONDS = 1.2f; // Time to paint the pattern
		static constexpr float SHINE_START = 10.3f; // When the shine starts
		static constexpr float SHINE_SECONDS = 1.6f; // Time the shine takes to cross the matrix
		static constexpr int EGG_COUNT = 5;
		static constexpr int EGG_WIDTH = 7; // Widest row of an egg
		static constexpr int EGG_HEIGHT = 10;
		static constexpr int EGG_TOP = 1; // Row of the top of the eggs
		static constexpr int EGG_STEP = 13; // Columns between two eggs
		static constexpr int EGG_LEFT = 6; // Column of the first egg
		static constexpr int PATTERN_COUNT = 5; // Stripes, zigzags, dots, vertical stripes, waves
		static constexpr uint8_t ROW_WIDTHS[EGG_HEIGHT] = { 3, 5, 5, 5, 7, 7, 7, 7, 5, 3 }; // Width of each row of an egg
		static constexpr Color BLANK = { 55, 50, 62 }; // An egg waiting for its paint

		Color base[EGG_COUNT] = {}; // Base color of each egg
		Color accent[EGG_COUNT] = {}; // Color of the pattern of each egg
		int pattern[EGG_COUNT] = {}; // Pattern of each egg
		float loopTime = 0; // Seconds into the loop

		// Tell if a pixel of an egg (column and row inside the egg) is part of a pattern
		static bool isPattern(int kind, int cx, int ry) {
			const int zig = abs(cx % 4 - 2);
			switch (kind) {
				case 0:
					return ry == 3 || ry == 6 || ry == 7;
				case 1:
					return ry == 3 + zig || ry == 6 + zig;
				case 2:
					return (ry == 3 || ry == 7) ? cx % 2 == 1 : (ry == 5 && cx % 2 == 0);
				case 3:
					return cx % 2 == 1 && ry >= 2 && ry <= 8;
				default:
					return ry == 4 + cx % 2 || ry == 8 - cx % 2;
			}
		}

		// Tell if a pixel is inside the egg shape
		static bool isInside(int cx, int ry) {
			return abs(cx - EGG_WIDTH / 2) <= ROW_WIDTHS[ry] / 2;
		}

		// Draw an egg: blank, then the base color coming down from the top, then the pattern coming from the left, then the shine
		void drawEgg(Frame &f, int i, float t, float fade, float shine, bool isShining) const {
			// Get how much of the base color and of the pattern are painted
			const float local = loopTime - i * SLOT_SECONDS;
			const float baseDone = constrain(local / BASE_SECONDS, 0.0f, 1.0f);
			const float patternDone = constrain((local - BASE_SECONDS) / PATTERN_SECONDS, 0.0f, 1.0f);
			const int left = EGG_LEFT + i * EGG_STEP;

			// Draw every pixel of the egg
			for (int ry = 0; ry < EGG_HEIGHT; ry++) {
				for (int cx = 0; cx < EGG_WIDTH; cx++) {
					if (!isInside(cx, ry)) {
						continue;
					}

					// Pick the color: blank, painted, or part of the pattern
					Color c = BLANK;
					if (ry < baseDone * EGG_HEIGHT + 0.5f && local >= 0) {
						c = base[i];
						if (isPattern(pattern[i], cx, ry) && cx < patternDone * EGG_WIDTH) {
							c = accent[i];
						}
					}

					// Add the shine band crossing the egg and a few twinkles once it has passed
					const int x = left + cx, y = EGG_TOP + ry;
					const float distance = fabsf(x + ry * 0.8f - shine);
					if (isShining && local > 0 && distance < 2.5f) {
						c = blendColor(c, WHITE, (1.0f - distance / 2.5f) * 0.6f);
					} else if (isShining && shine > x + 12 && sinf(t * 6 + x * 3.1f + y * 5.7f) > 0.97f) {
						c = blendColor(c, WHITE, 0.7f);
					}
					setPixel(f, x, y, scaleColor(c, fade));
				}
			}
		}

		// Draw the brush: it scribbles down the egg while the base color goes on, then follows the pattern from left to right
		void drawBrush(Frame &f, int i, float local) const {
			const int left = EGG_LEFT + i * EGG_STEP;
			int x, y;
			Color tip;
			if (local < BASE_SECONDS) {
				// Scribble down the egg
				const float done = local / BASE_SECONDS;
				x = left + 3 + (int)lroundf(2.0f * sinf(done * 22));
				y = EGG_TOP + min(EGG_HEIGHT - 1, (int)(done * EGG_HEIGHT));
				tip = base[i];
			} else {
				// Follow the pattern: the column being painted, at the first row of the pattern in it
				const int cx = min(EGG_WIDTH - 1, (int)((local - BASE_SECONDS) / PATTERN_SECONDS * EGG_WIDTH));
				int ry = 4;
				for (int r = 0; r < EGG_HEIGHT; r++) {
					if (isInside(cx, r) && isPattern(pattern[i], cx, r)) {
						ry = r;
						break;
					}
				}
				x = left + cx;
				y = EGG_TOP + ry;
				tip = accent[i];
			}

			// Draw the handle going up to the right, then the tip
			setPixel(f, x + 1, y - 1, { 210, 210, 220 });
			setPixel(f, x + 2, y - 2, { 170, 110, 60 });
			setPixel(f, x + 3, y - 3, { 170, 110, 60 });
			setPixel(f, x, y, tip);
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getPaintedEggsScene() {
	static PaintedEggsScene scene;
	return scene;
}
