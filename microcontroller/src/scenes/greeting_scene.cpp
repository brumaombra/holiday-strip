#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"
#include "font/font.h"
#include "calendar/countdown.h"

namespace {

	// Scrolling greeting between two chasing garlands, with a countdown to Christmas
	class GreetingScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "greeting"; }
		const char *name() const override { return "Greeting"; }

		// Build the first text and restart the scroll
		void begin() override {
			text = buildText();
			scroll = 0;
		}

		// Draw the scrolling text between the two chasing garlands
		void draw(Frame &f, float t, float dt) override {
			// Draw the starry background
			fillFrame(f, { 0, 0, 0 });
			stars.draw(f, t);

			// Black out the rows around the text so it stands out from the stars
			for (int y : { 1, 9, 10 }) {
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = { 0, 0, 0 };
				}
			}

			// Define the colors the letters take in turn
			const Color palette[] = { RED, GREEN, GOLD };

			// Scroll the text left and restart it once it has left the screen
			// Each pass picks up the latest message and countdown
			scroll += SPEED * dt;
			if (scroll >= textWidth(text) + WIDTH) {
				scroll = 0;
				text = buildText();
			}

			// Draw the letters that are on the screen
			float pos = WIDTH - scroll;
			for (size_t i = 0; i < text.length(); i++) {
				int x = (int)pos + i * GLYPH_ADVANCE;

				// Skip the letters outside the screen
				if (x < -GLYPH_ADVANCE || x >= WIDTH) {
					continue;
				}

				// Pick the color of the letter
				Color base = palette[i % 3];

				// Draw the letter with a wave of brightness running through it
				drawChar(f, text[i], x, 2, [&](int px, int py) {
					return scaleColor(base, 0.75f + 0.25f * sinf(px * 0.4f - t * 6.0f));
				});
			}

			// Light every third bulb of the two garlands, 5 steps per second
			int step = (int)(t * 5);
			for (int x = 1; x < WIDTH; x += 3) {
				int bulb = x / 3;
				Color c = LIGHT_COLORS[bulb % LIGHT_COLOR_COUNT];

				// Draw the top bulb and the bottom one (they chase in opposite directions)
				setPixel(f, x, 0, scaleColor(c, (bulb + step) % 3 == 0 ? 1.0f : 0.25f));
				setPixel(f, WIDTH - 1 - x, HEIGHT - 1, scaleColor(c, (bulb + step) % 3 == 0 ? 1.0f : 0.25f));
			}
		}

		// Place the stars once
		GreetingScene() { stars.begin(); }

	private:
		static constexpr float SPEED = 16.0f; // Scroll speed in columns per second
		String text; // Message being scrolled
		float scroll = 0; // Columns scrolled so far in this pass
		Stars stars;

		// Build the scrolling text: the message, then the Christmas countdown when it's enabled and the clock is set
		static String buildText() {
			// Start from the greeting message
			String result = getHolidayMessage(CHRISTMAS);

			// Stop here when the countdown is turned off
			if (!isCountdownEnabled()) {
				return result;
			}

			// Add the countdown, if there is one today
			struct tm now;
			char countdown[48]; // Room for the countdown text
			if (getLocalTime(&now, 0) && formatChristmasCountdown(now, countdown, sizeof(countdown), getMatrixLanguage())) {
				result += countdown;
			}

			// Return the message with the countdown when there is one
			return result;
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getGreetingScene() {
	static GreetingScene scene;
	return scene;
}