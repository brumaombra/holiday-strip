#include "scene_list.h"

#include <time.h>

#include "calendar/countdown.h"
#include "font/font.h"
#include "scenes/effects.h"

namespace {

	// The countdown to Christmas between two little trees in the snow: the number of days, then the sentence scrolling by, in the language of the matrix
	class ChristmasCountdownScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "christmas-countdown"; }
		const char *name() const override { return "Countdown to Christmas"; }

		// Start with the number of days, and the snow spread over the screen
		void begin() override {
			snow.begin(20);
			startNumber(0);
		}

		// Draw the snow, the trees, and the number or the sentence between them
		void draw(Frame &f, float t, float dt) override {
			// The night, the snow behind, and the two trees
			fillFrame(f, { 0, 0, 6 });
			snow.update(t, dt, nullptr);
			snow.draw(f, t);
			drawTree(f, LEFT_TREE, t);
			drawTree(f, RIGHT_TREE, t);

			// The number of days for a while, then the sentence once across, then again
			if (isShowingNumber) {
				drawNumber(f, t);
				if (t - phaseStart >= NUMBER_SECONDS && days >= 0) {
					startSentence(t);
				}
			} else {
				scroll += SPEED * dt;
				drawSentence(f, t);
				if (scroll >= textWidth(sentence) + WINDOW_WIDTH) {
					startNumber(t);
				}
			}
		}

	private:
		static constexpr float SPEED = 16.0f; // Scroll speed in columns per second
		static constexpr float NUMBER_SECONDS = 4.0f; // How long the number of days shows
		static constexpr int LEFT_TREE = 6, RIGHT_TREE = WIDTH - 7; // Center columns of the trees
		static constexpr int WINDOW_LEFT = 13, WINDOW_WIDTH = WIDTH - 26; // The space between the trees, where the text goes

		Snowfall snow;
		bool isShowingNumber = true; // The number of days, or the sentence scrolling
		float phaseStart = 0; // Scene time when the number started showing
		float scroll = 0; // Columns the sentence has scrolled
		long days = -1; // Days to Christmas (-1 while the clock isn't set)
		String sentence; // The countdown sentence being scrolled

		// Read the days to Christmas (-1 while the clock isn't set)
		static long readDaysToChristmas() {
			struct tm now = {};
			return getLocalTime(&now, 0) ? getDaysToChristmas(now) : -1;
		}

		// Show the number of days, read again now
		void startNumber(float t) {
			days = readDaysToChristmas();
			isShowingNumber = true;
			phaseStart = t;
		}

		// Scroll the sentence, written in the language of the matrix
		void startSentence(float t) {
			char text[48]; // Room for the longest sentence
			formatChristmasCountdown(days, getMatrixLanguage(), text, sizeof(text));
			sentence = text;
			scroll = 0;
			isShowingNumber = false;
		}

		// Draw a little tree: a green triangle from row 2 to 9 with twinkling ornaments, a gold star on top and a trunk
		static void drawTree(Frame &f, int cx, float t) {
			for (int y = 2; y <= 9; y++) {
				const int halfWidth = (y - 1) / 2 + (y > 5 ? 1 : 0);
				for (int x = cx - halfWidth; x <= cx + halfWidth; x++) {
					setPixel(f, x, y, { 0, 110, 25 });
				}
			}

			// Ornaments, each twinkling in its own color
			const int ornaments[][2] = { { -1, 5 }, { 1, 4 }, { 0, 7 }, { -3, 9 }, { 2, 8 }, { 3, 9 } };
			for (int i = 0; i < 6; i++) {
				const Color color = LIGHT_COLORS[(i + cx) % LIGHT_COLOR_COUNT];
				setPixel(f, cx + ornaments[i][0], ornaments[i][1], scaleColor(color, 0.4f + 0.6f * twinkle(t, 3.0f, i * 1.7f + cx)));
			}

			// The star and the trunk
			setPixel(f, cx, 1, scaleColor(GOLD, 0.7f + 0.3f * twinkle(t, 5.0f, cx)));
			setPixel(f, cx, 10, { 110, 50, 10 });
			setPixel(f, cx, 11, { 110, 50, 10 });
		}

		// Draw the number of days in the middle, gold with a glint running through it (dots while the clock isn't set, a heart on Christmas day)
		void drawNumber(Frame &f, float t) {
			const String text = days < 0 ? String("...") : (days == 0 ? String("<") : String(days));
			const int x0 = (WIDTH - textWidth(text) + 1) / 2;
			const float glint = fmodf(t, 2.0f) / 2.0f * (textWidth(text) + 10) - 5 + x0;
			for (size_t i = 0; i < text.length(); i++) {
				drawChar(f, text[i], x0 + i * GLYPH_ADVANCE, 3, [&](int px, int py) {
					const Color base = days == 0 ? RED : GOLD;
					return blendColor(base, WHITE, max(0.0f, 1.0f - fabsf(px - glint) / 3.0f) * 0.7f);
				});
			}
		}

		// Draw the sentence scrolling through the space between the trees, in red, green and gold letters
		void drawSentence(Frame &f, float t) const {
			const Color palette[] = { RED, GREEN, GOLD };
			const float pos = WINDOW_LEFT + WINDOW_WIDTH - scroll;
			for (size_t i = 0; i < sentence.length(); i++) {
				const int x = (int)pos + i * GLYPH_ADVANCE;
				if (x < WINDOW_LEFT - GLYPH_ADVANCE || x >= WINDOW_LEFT + WINDOW_WIDTH) {
					continue;
				}

				// Draw the letter from the font, only the columns inside the window
				const Glyph *glyph = findGlyph(sentence[i]);
				if (glyph == nullptr) {
					continue;
				}
				for (int col = 0; col < 5; col++) {
					const int px = x + col;
					if (px < WINDOW_LEFT || px >= WINDOW_LEFT + WINDOW_WIDTH) {
						continue;
					}
					for (int row = 0; row < 7; row++) {
						if (glyph->cols[col] >> row & 1) {
							setPixel(f, px, 3 + row, palette[i % 3]);
						}
					}
				}
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getChristmasCountdownScene() {
	static ChristmasCountdownScene scene;
	return scene;
}