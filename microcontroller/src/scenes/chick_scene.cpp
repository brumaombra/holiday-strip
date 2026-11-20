#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// An egg that wobbles and cracks, its top pops off and a chick peeks out, blinks and cheeps (a 15 second loop)
	class ChickScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "chick"; }
		const char *name() const override { return "Hatching chick"; }

		// Restart the hatching
		void begin() override {
			loopTime = 0;
			isPopped = false;
			for (auto &shard : shards) {
				shard.life = 0;
			}
		}

		// Draw the egg, the chick behind its shell, the flying shell and the shards
		void draw(Frame &f, float t, float dt) override {
			// Run the loop
			loopTime += dt;
			if (loopTime >= LOOP_SECONDS) {
				begin();
			}

			// Draw the dawn sky and the grass
			drawBackground(f);

			// Get the wobble of the egg, a quick sway during three short moments
			int sway = 0;
			for (const float start : { 1.0f, 2.6f, 4.0f }) {
				if (loopTime >= start && loopTime < start + 0.8f) {
					sway = (int)lroundf(sinf((loopTime - start) * 28.0f));
				}
			}

			// Pop the top off, throwing a few shards
			if (!isPopped && loopTime >= POP_AT) {
				isPopped = true;
				throwShards();
			}

			// Get how much of the egg is cracked
			const float cracked = constrain((loopTime - CRACK_START) / (CRACK_END - CRACK_START), 0.0f, 1.0f);

			// Draw the egg whole until it pops
			if (!isPopped) {
				drawEgg(f, EGG_LEFT + sway, true, true, cracked);
			} else {
				// Draw the chick rising from the bottom of the shell, then the bottom of the shell over it
				drawChick(f);
				drawEgg(f, EGG_LEFT, false, true, 0.0f);

				// Draw the top of the shell flying away up and to the right
				const float flight = loopTime - POP_AT;
				if (flight < 1.0f) {
					drawEgg(f, EGG_LEFT + (int)(flight * 8), true, false, 0.0f, (int)lroundf(-26 * flight + 12 * flight * flight));
				}
			}

			// Move and draw the shards
			for (auto &shard : shards) {
				if (shard.life <= 0) {
					continue;
				}
				shard.life -= dt;
				shard.vy += 30.0f * dt; // Gravity
				shard.x += shard.vx * dt;
				shard.y += shard.vy * dt;
				setPixel(f, (int)shard.x, (int)shard.y, SHELL);
			}

			// Fade out at the end of the loop
			const float fade = constrain((LOOP_SECONDS - loopTime) / FADE_SECONDS, 0.0f, 1.0f);
			if (fade < 1.0f) {
				for (auto &row : f) {
					for (auto &p : row) {
						p = scaleColor(p, fade);
					}
				}
			}
		}

	private:
		static constexpr float LOOP_SECONDS = 15.2f;
		static constexpr float FADE_SECONDS = 1.0f; // Fade at the end of the loop
		static constexpr float CRACK_START = 1.8f; // When the first crack appears
		static constexpr float CRACK_END = 4.6f; // When the crack has gone all around
		static constexpr float POP_AT = 5.0f; // When the top flies off
		static constexpr float RISE_SECONDS = 1.6f; // Time the chick takes to rise
		static constexpr int EGG_LEFT = 32; // Column of the left of the egg
		static constexpr int EGG_WIDTH = 9;
		static constexpr int EGG_HEIGHT = 11;
		static constexpr uint8_t ROW_WIDTHS[EGG_HEIGHT] = { 3, 5, 7, 7, 9, 9, 9, 9, 7, 5, 3 }; // Width of each row of the egg
		static constexpr int MAX_SHARDS = 6;
		static constexpr Color SHELL = { 232, 222, 196 };
		static constexpr Color YELLOW = { 255, 215, 30 };
		static constexpr Color DARK_YELLOW = { 220, 140, 0 };
		static constexpr Color BEAK = { 255, 100, 20 };
		static constexpr Color EYE = { 60, 30, 0 }; // Dark brown, still lit

		// A shard of shell
		struct Shard {
			float x, y, vx, vy, life;
		};

		// The chick facing right (y yellow, W wing, k eye, o beak): a round head with a tuft, and its upper body
		static constexpr const char *CHICK[] = {
			"...yy.....",
			"..yyyyyy..",
			".yyyyyyyy.",
			".yyykyyky.",
			".yyykyykoo",
			".yWWyyyyyo",
			".yWWyyyyy.",
			"..yyyyyy..",
		};

		Shard shards[MAX_SHARDS] = {};
		float loopTime = 0; // Seconds into the loop
		bool isPopped = false; // Tells if the top has flown off already

		// Get the row where the shell is cut, jagged like a crack (the top shell is above it)
		static int cutRow(int cx) {
			return 8 - (cx & 1);
		}

		// Tell if a pixel is inside the egg shape
		static bool isInside(int cx, int ry) {
			return abs(cx - EGG_WIDTH / 2) <= ROW_WIDTHS[ry] / 2;
		}

		// Draw the sky, getting warmer toward the horizon, and the grass
		static void drawBackground(Frame &f) {
			for (int y = 0; y < HEIGHT; y++) {
				const Color row = blendColor({ 10, 10, 38 }, { 70, 45, 75 }, y / 10.0f);
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = row;
				}
			}
			for (int x = 0; x < WIDTH; x++) {
				setPixel(f, x, 11, { 14, 90, 30 });
				if ((x * 7 + 3) % 5 < 2) {
					setPixel(f, x, 10, { 25, 120, 40 });
				}
			}
		}

		// Draw the top and/or the bottom of the egg (with an optional crack along the cut, and an offset for the flying top)
		static void drawEgg(Frame &f, int left, bool isTop, bool isBottom, float crack, int up = 0) {
			for (int ry = 0; ry < EGG_HEIGHT; ry++) {
				for (int cx = 0; cx < EGG_WIDTH; cx++) {
					if (!isInside(cx, ry)) {
						continue;
					}

					// Skip the part of the egg that isn't drawn
					const bool isUpper = ry < cutRow(cx);
					if (isUpper ? !isTop : !isBottom) {
						continue;
					}

					// Pick the color: a lilac band on the top, a pink one on the bottom, a few speckles, and the crack
					Color c = SHELL;
					if (isUpper && ry == 2) {
						c = { 190, 150, 255 };
					} else if (!isUpper && ry == 9) {
						c = { 255, 170, 200 };
					} else if ((cx * 7 + ry * 13) % 19 == 0) {
						c = { 150, 110, 80 };
					}
					if (isTop && isBottom && ry == cutRow(cx) && cx < crack * EGG_WIDTH) {
						c = { 50, 25, 15 };
					}
					setPixel(f, left + cx, ry + up, c);
				}
			}
		}

		void drawChick(Frame &f) const {
			// Get the height of the chick: it rises, easing out
			const float rise = constrain((loopTime - POP_AT - 0.2f) / RISE_SECONDS, 0.0f, 1.0f);
			int top = 1 + (int)lroundf((1.0f - rise) * (1.0f - rise) * 11);

			// Cheep twice (a small hop, an open beak, flapping wings and sound marks), and blink three times
			bool isCheeping = false, isBlinking = false;
			for (const float start : { 9.0f, 12.0f }) {
				isCheeping = isCheeping || (loopTime >= start && loopTime < start + 0.7f);
			}
			for (const float start : { 7.6f, 10.5f, 13.4f }) {
				isBlinking = isBlinking || (loopTime >= start && loopTime < start + 0.25f);
			}
			isCheeping = isCheeping && rise >= 1.0f;
			if (isCheeping) {
				top -= 1;
			}

			// Draw the chick
			const int left = EGG_LEFT;
			for (int r = 0; r < 8; r++) {
				for (int c = 0; c < 10; c++) {
					const char px = CHICK[r][c];
					if (px == '.') {
						continue;
					}
					Color color = YELLOW;
					if (px == 'k') {
						color = isBlinking ? DARK_YELLOW : EYE;
					} else if (px == 'o') {
						color = BEAK;
					} else if (px == 'W') {
						color = DARK_YELLOW;
					}
					setPixel(f, left + c, top + r, color);
				}
			}

			// Flap the wings out, open the beak and send out the sound while cheeping
			if (isCheeping) {
				const bool isUp = (int)(loopTime * 12) % 2 == 0;
				setPixel(f, left, top + (isUp ? 3 : 5), DARK_YELLOW);
				setPixel(f, left - 1, top + (isUp ? 2 : 4), DARK_YELLOW);
				setPixel(f, left, top + 4, DARK_YELLOW);
				setPixel(f, left + 8, top + 5, BEAK);
				setPixel(f, left + 9, top + 6, BEAK);
				setPixel(f, left + 11, top + 3, { 255, 240, 160 });
				setPixel(f, left + 12, top + 4, { 255, 240, 160 });
				setPixel(f, left + 11, top + 6, { 255, 240, 160 });
			}
		}

		// Throw shards of shell out of the egg
		void throwShards() {
			for (auto &shard : shards) {
				shard = { EGG_LEFT + 4.0f, 7.0f, frand(-14, 14), frand(-16, -6), frand(0.5f, 1.0f) };
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getChickScene() {
	static ChickScene scene;
	return scene;
}
