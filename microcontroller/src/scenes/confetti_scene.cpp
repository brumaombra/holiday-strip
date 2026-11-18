#include "scene_list.h"

#include "font/font.h"
#include "scenes/effects.h"

namespace {

	// The New Year's Eve greeting scrolling under falling confetti
	class ConfettiScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "confetti"; }
		const char *name() const override { return "Confetti"; }

		// Take the latest greeting, restart the scroll and spread the confetti over the screen
		void begin() override {
			text = getHolidayMessage(NEW_YEAR);
			scroll = 0;
			for (auto &piece : pieces) {
				respawn(piece, true);
			}
		}

		// Draw the confetti, with the greeting scrolling over it
		void draw(Frame &f, float t, float dt) override {
			// Start from a deep night blue
			fillFrame(f, { 2, 0, 10 });

			// Move and draw the confetti (each piece flashes as it spins)
			for (auto &piece : pieces) {
				piece.y += piece.speed * dt;
				if (piece.y >= HEIGHT) {
					respawn(piece, false);
				}
				const int x = (int)lroundf(piece.x + sinf(t * piece.flutter + piece.phase) * 1.2f); // Flutter left and right
				const float spin = 0.25f + 0.75f * fabsf(sinf(t * piece.spin + piece.phase));
				if (piece.y >= 0) {
					setPixel(f, x, (int)piece.y, scaleColor(piece.color, spin));
				}
			}

			// Scroll the greeting left and restart it, with the latest message, once it has left the screen
			scroll += SPEED * dt;
			if (scroll >= textWidth(text) + WIDTH) {
				scroll = 0;
				text = getHolidayMessage(NEW_YEAR);
			}

			// Draw the letters on the screen in gold and silver, a sparkle running through them
			const float pos = WIDTH - scroll;
			for (size_t i = 0; i < text.length(); i++) {
				const int x = (int)pos + i * GLYPH_ADVANCE;
				if (x < -GLYPH_ADVANCE || x >= WIDTH) {
					continue;
				}
				const Color base = i % 2 ? SILVER : GOLD;
				drawChar(f, text[i], x, 2, [&](int px, int py) {
					return blendColor(base, WHITE, max(0.0f, sinf(px * 0.5f - t * 7.0f)) * 0.5f);
				});
			}
		}

	private:
		static constexpr float SPEED = 16.0f; // Scroll speed in columns per second
		static constexpr int PIECE_COUNT = 44; // Confetti on the screen at once
		static constexpr Color SILVER = { 190, 200, 215 };
		static constexpr Color CONFETTI_COLORS[] = { GOLD, SILVER, PINK, { 0, 200, 255 }, { 170, 60, 255 }, { 60, 255, 120 } };

		// A piece of confetti
		struct Piece {
			float x, y, speed, flutter, spin, phase; // Position, fall speed (rows per second), flutter and spin speeds, and their phase
			Color color;
		};

		Piece pieces[PIECE_COUNT] = {};
		String text; // Greeting being scrolled
		float scroll = 0; // Columns scrolled so far in this pass

		// Put a piece back at the top (anywhere also spreads it over the screen, for the start)
		static void respawn(Piece &piece, bool anywhere) {
			piece.x = frand(0, WIDTH);
			piece.y = anywhere ? frand(-HEIGHT, HEIGHT) : frand(-3, -0.5f);
			piece.speed = frand(1.5f, 3.5f);
			piece.flutter = frand(2.0f, 4.0f);
			piece.spin = frand(4.0f, 9.0f);
			piece.phase = frand(0, 6.28f);
			piece.color = CONFETTI_COLORS[esp_random() % (sizeof(CONFETTI_COLORS) / sizeof(CONFETTI_COLORS[0]))];
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getConfettiScene() {
	static ConfettiScene scene;
	return scene;
}