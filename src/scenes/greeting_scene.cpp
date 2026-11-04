#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"
#include "font/font.h"
#include "calendar/countdown.h"

namespace {

	// Scrolling greeting between two chasing garlands, with a countdown to Christmas
	class GreetingScene : public Scene {
	public:
		const char *name() const override { return "Greeting"; }

		void begin() override {
			text = buildText();
			scroll = 0;
		}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, {0, 0, 0});
			stars.draw(f, t);
			for (int y : {1, 9, 10}) {
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = {0, 0, 0};
				}
			}

			const Color palette[] = {RED, GREEN, GOLD};
			// Each pass picks up the latest message and countdown
			scroll += SPEED * dt;
			if (scroll >= textWidth(text) + WIDTH) {
				scroll = 0;
				text = buildText();
			}
			float pos = WIDTH - scroll;
			for (size_t i = 0; i < text.length(); i++) {
				int x = (int)pos + i * GLYPH_ADVANCE;
				if (x < -GLYPH_ADVANCE || x >= WIDTH) {
					continue;
				}
				Color base = palette[i % 3];
				drawChar(f, text[i], x, 2, [&](int px, int py) {
					return scaleColor(base, 0.75f + 0.25f * sinf(px * 0.4f - t * 6.0f));
				});
			}

			int step = (int)(t * 5);
			for (int x = 1; x < WIDTH; x += 3) {
				int bulb = x / 3;
				Color c = LIGHT_COLORS[bulb % LIGHT_COLOR_COUNT];
				setPixel(f, x, 0, scaleColor(c, (bulb + step) % 3 == 0 ? 1.0f : 0.25f));
				setPixel(f, WIDTH - 1 - x, HEIGHT - 1, scaleColor(c, (bulb + step) % 3 == 0 ? 1.0f : 0.25f));
			}
		}

		GreetingScene() { stars.begin(); }

	private:
		static constexpr float SPEED = 16.0f;
		String text;
		float scroll = 0;
		Stars stars;

		// Build the scrolling text: the message, then the Christmas countdown when it's enabled and the clock is set
		static String buildText() {
			String result = getGreetingMessage();
			if (!isCountdownEnabled()) {
				return result;
			}

			// Add the countdown, if there is one today
			struct tm now;
			char countdown[48];
			if (getLocalTime(&now, 0) && formatChristmasCountdown(now, countdown, sizeof(countdown))) {
				result += countdown;
			}
			return result;
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getGreetingScene() {
	static GreetingScene scene;
	return scene;
}