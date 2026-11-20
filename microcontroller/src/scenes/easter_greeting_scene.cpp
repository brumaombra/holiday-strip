#include "scene_list.h"

#include "font/font.h"
#include "scenes/effects.h"

namespace {

	// The Easter greeting scrolling over a meadow of spring flowers: each one grows, opens, then shrinks and grows again in a new color
	class EasterGreetingScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "easter-greeting"; }
		const char *name() const override { return "Greeting"; }

		// Take the latest message and restart the scroll
		void begin() override {
			text = getHolidayMessage(EASTER);
			scroll = 0;
		}

		// Draw the meadow with its flowers, then the scrolling greeting on top
		void draw(Frame &f, float t, float dt) override {
			// Draw the soft sky, getting a little greener toward the meadow
			for (int y = 0; y < HEIGHT; y++) {
				const Color row = blendColor({ 6, 6, 26 }, { 8, 36, 26 }, y / (float)(HEIGHT - 1));
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = row;
				}
			}

			// Draw the grass and the flowers
			drawGrass(f, t);
			for (int i = 0; i < FLOWER_COUNT; i++) {
				drawFlower(f, i, t);
			}

			// Scroll the text left and restart it, with the latest message, once it has left the screen
			scroll += SPEED * dt;
			if (scroll >= textWidth(text) + WIDTH) {
				scroll = 0;
				text = getHolidayMessage(EASTER);
			}

			// Draw the letters that are on the screen, each in a pastel color with a wave of brightness running through it
			const float pos = WIDTH - scroll;
			for (size_t i = 0; i < text.length(); i++) {
				const int x = (int)pos + i * GLYPH_ADVANCE;
				if (x < -GLYPH_ADVANCE || x >= WIDTH) {
					continue;
				}
				const Color base = PETALS[i % PETAL_COUNT];
				drawChar(f, text[i], x, 1, [&](int px, int py) { return scaleColor(base, 0.8f + 0.2f * sinf(px * 0.4f - t * 5.0f)); });
			}
		}

	private:
		static constexpr float SPEED = 16.0f; // Scroll speed in columns per second
		static constexpr int FLOWER_COUNT = 9; // Flowers along the bottom, one every 8 columns
		static constexpr float CYCLE_SECONDS = 9.0f; // Life of a flower, from the seed to the next seed
		static constexpr float GROW_SECONDS = 1.6f; // Time the stem takes to grow
		static constexpr float OPEN_SECONDS = 1.0f; // Time the bud takes to open
		static constexpr float SHRINK_SECONDS = 0.8f; // Time the flower takes to go back into the grass
		static constexpr Color PETALS[] = { { 255, 110, 170 }, { 190, 130, 255 }, { 110, 190, 255 }, { 255, 160, 100 }, { 240, 235, 255 }, { 255, 100, 100 } }; // Pink, lilac, sky blue, peach, white, coral
		static constexpr int PETAL_COUNT = sizeof(PETALS) / sizeof(PETALS[0]); // Number of petal colors
		static constexpr Color STEM = { 30, 170, 50 }; // Stems and leaves
		static constexpr Color CENTER = { 255, 200, 20 }; // Heart of the flower

		String text; // Message being scrolled
		float scroll = 0; // Columns scrolled so far in this pass

		// Draw the two rows of grass at the bottom, with a few blades poking up
		static void drawGrass(Frame &f, float t) {
			for (int x = 0; x < WIDTH; x++) {
				setPixel(f, x, 11, { 12, 90, 28 });
				const int blade = (x * 7 + 3) % 5;
				if (blade < 2) {
					setPixel(f, x, 10, blade == 0 ? Color{ 25, 130, 40 } : Color{ 15, 100, 32 });
				}
			}
		}

		// Draw one flower, whose life is shifted from the others so they don't bloom together
		static void drawFlower(Frame &f, int i, float t) {
			// Get where the flower is in its life, and its color for this round
			const float local = t + i * 1.7f;
			const int round = (int)(local / CYCLE_SECONDS);
			const float age = fmodf(local, CYCLE_SECONDS);
			const Color petal = PETALS[(i * 5 + round * 3) % PETAL_COUNT];

			// Get the place of the head: every third flower is taller
			const int x = 4 + i * 8 + (i * 5) % 3 - 1;
			const int top = (i % 3 == 0) ? 8 : 9;

			// Get how grown, open and alive the flower is
			const float grown = constrain(age / GROW_SECONDS, 0.0f, 1.0f);
			const float open = constrain((age - GROW_SECONDS) / OPEN_SECONDS, 0.0f, 1.0f);
			const float alive = constrain((CYCLE_SECONDS - age) / SHRINK_SECONDS, 0.0f, 1.0f);

			// Draw the stem, growing up from the grass (and sinking back at the end)
			const int fullStem = 11 - top;
			const int stem = (int)lroundf(fullStem * min(grown, alive));
			for (int k = 0; k < stem; k++) {
				setPixel(f, x, 11 - k, STEM);
			}

			// Draw a leaf once the stem is tall enough
			if (stem >= 2) {
				setPixel(f, x + (i % 2 ? 1 : -1), 10, { 20, 140, 40 });
			}

			// Draw the head once the stem is done: a bud first, then the petals around the center
			if (stem < fullStem) {
				return;
			}
			if (open < 0.5f) {
				setPixel(f, x, top, scaleColor(petal, 0.6f));
				return;
			}
			const Color shown = scaleColor(petal, alive);
			setPixel(f, x - 1, top, shown);
			setPixel(f, x + 1, top, shown);
			setPixel(f, x, top - 1, shown);
			setPixel(f, x, top + 1, shown);

			// Fill in the corners of the head once fully open
			if (open >= 1.0f) {
				const Color corner = scaleColor(petal, alive * 0.55f);
				setPixel(f, x - 1, top - 1, corner);
				setPixel(f, x + 1, top - 1, corner);
				setPixel(f, x - 1, top + 1, corner);
				setPixel(f, x + 1, top + 1, corner);
			}
			setPixel(f, x, top, scaleColor(CENTER, 0.75f + 0.25f * sinf(t * 3 + i)));
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getEasterGreetingScene() {
	static EasterGreetingScene scene;
	return scene;
}
