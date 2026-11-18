#include "scene_list.h"

#include "font/font.h"
#include "scenes/effects.h"

namespace {

	// The custom message scrolling on black, in the color picked on the dashboard (or a rainbow)
	class CustomMessageScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "message"; }
		const char *name() const override { return "Message"; }

		// Take the latest message and restart the scroll
		void begin() override {
			text = getHolidayMessage(CUSTOM);
			scroll = 0;
		}

		// Draw the letters on the screen in the color of the message
		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, { 0, 0, 0 });

			// Scroll left and restart, with the latest message, once it has left the screen
			scroll += SPEED * dt;
			if (scroll >= textWidth(text) + WIDTH) {
				scroll = 0;
				text = getHolidayMessage(CUSTOM);
			}

			// Draw the letters that are on the screen
			const uint8_t color = getMessageColor();
			const float pos = WIDTH - scroll;
			for (size_t i = 0; i < text.length(); i++) {
				const int x = (int)pos + i * GLYPH_ADVANCE;
				if (x < -GLYPH_ADVANCE || x >= WIDTH) {
					continue;
				}
				drawChar(f, text[i], x, 2, [&](int px, int py) { return letterColor(color, px, t); });
			}
		}

	private:
		static constexpr float SPEED = 16.0f; // Scroll speed in columns per second
		static constexpr uint8_t RAINBOW = 6; // The last color: every column its own hue, drifting over time
		static constexpr Color COLORS[] = { WHITE, WARM_WHITE, RED, { 0, 220, 40 }, BLUE, GOLD }; // White, warm white, red, green, blue, gold

		String text; // Message being scrolled
		float scroll = 0; // Columns scrolled so far in this pass

		// Get the color of a column of the message
		static Color letterColor(uint8_t color, int x, float t) {
			if (color == RAINBOW) {
				return hsvColor((uint8_t)(x * 5 + t * 60), 255, 255);
			}
			return COLORS[color < RAINBOW ? color : 0];
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getCustomMessageScene() {
	static CustomMessageScene scene;
	return scene;
}