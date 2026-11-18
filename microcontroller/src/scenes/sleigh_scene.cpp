#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Santa's sleigh and reindeer flying across a starry sky, leaving a trail of sparkles
	class SleighScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "sleigh"; }
		const char *name() const override { return "Santa's sleigh"; }

		// Place the stars once
		SleighScene() { stars.begin(); }

		// Clear the sparkle trail
		void begin() override {
			for (auto &p : trail) {
				p.life = 0;
			}
		}

		// Draw the sleigh and the two reindeer flying over the sky, with a sparkle trail
		void draw(Frame &f, float t, float dt) override {
			// Draw the night sky with its stars and the moon
			fillFrame(f, { 0, 0, 12 });
			stars.draw(f, t);
			drawMoon(f);

			// Move the whole team to the right and wrap it around the matrix
			const int total = 38; // Width of the sleigh and the reindeer in columns
			float x0 = -total + fmodf(t * 14.0f, WIDTH + total + 12); // 14 columns per second, with a gap before the next pass
			int sx = (int)x0;

			// Bob the sleigh and the reindeer up and down, each a bit out of phase
			int sleighY = 1 + (int)lroundf(sinf(t * 3.0f) * 0.7f);
			int deer1Y = 2 + (int)lroundf(sinf(t * 3.0f + 1.0f) * 0.7f);
			int deer2Y = 2 + (int)lroundf(sinf(t * 3.0f + 2.0f) * 0.7f);

			// Switch the leg pose of the reindeer six times per second
			bool legs = ((int)(t * 6)) % 2;

			// Move the sparkles and draw them behind the sleigh
			updateTrail(dt, sx, sleighY);
			drawTrail(f);

			// Harness
			for (int x = sx + 12; x < sx + 16; x++) {
				setPixel(f, x, sleighY + 6, GOLD);
			}
			for (int x = sx + 25; x < sx + 28; x++) {
				setPixel(f, x, deer1Y + 4, GOLD);
			}

			// Draw the sleigh and the two reindeer (the second one is Rudolph)
			drawSprite(f, SLEIGH, 10, sx, sleighY, t);
			drawDeer(f, sx + 15, deer1Y, legs, false, t);
			drawDeer(f, sx + 28, deer2Y, !legs, true, t);
		}

	private:
		// A sparkle of the trail
		struct Particle {
			float x, y, life; // Position and remaining life in seconds
		};
		static constexpr int MAX_PARTICLES = 40; // Size of the trail pool
		Particle trail[MAX_PARTICLES];
		float spawnAccumulator = 0; // Fraction of a sparkle waiting to be spawned
		Stars stars;

		// Sprite of the sleigh with Santa on the left
		static constexpr const char *SLEIGH[10] = {
			"....w........",
			"...rr........",
			"..rrrr.......",
			"..wwww.......",
			"..sss........",
			"r.wwww......y",
			"rrrrrrrrrrr.y",
			"rryyyyyyyyrr.",
			".rrrrrrrrrr..",
			"yyyyyyyyyyyyy",
		};

		// Sprite of a reindeer body with its head on the right (the nose is red only for Rudolph)
		static constexpr const char *DEER_BODY[6] = {
			"......a.a.",
			".......a..",
			"......bbbn",
			"w.....bb..",
			"bbbbbbbb..",
			"bbbbbbb...",
		};

		// Sprites of the two leg poses, drawn under the body
		static constexpr const char *LEGS_A[2] = { ".b....b.b.", "b......b.b" };
		static constexpr const char *LEGS_B[2] = { "..b...b...", "..b...b..." };

		// Draw a reindeer (Rudolph gets a glowing red nose)
		void drawDeer(Frame &f, int x, int y, bool legPose, bool rudolph, float t) {
			// Draw the body row by row
			for (int r = 0; r < 6; r++) {
				for (int c = 0; DEER_BODY[r][c]; c++) {
					char ch = DEER_BODY[r][c];

					// Swap the red nose for brown fur when it isn't Rudolph
					if (ch == 'n' && !rudolph) {
						ch = 'b';
					}

					// Draw the pixel unless it's transparent
					if (ch != '.') {
						setPixel(f, x + c, y + r, spriteColor(ch, t));
					}
				}
			}

			// Draw the legs under the body in the current pose
			drawSprite(f, legPose ? LEGS_A : LEGS_B, 2, x, y + 6, t);
		}

		// Draw a crescent moon in the top right corner (a disc minus a shifted disc)
		static void drawMoon(Frame &f) {
			// Check each pixel of the area around the moon
			for (int y = 0; y < 7; y++) {
				for (int x = 60; x < 72; x++) {
					// Get the distance to the center of each disc
					float d1 = hypotf(x - 66.0f, y - 3.0f);
					float d2 = hypotf(x - 67.6f, y - 2.2f);

					// Draw the pixels inside the big disc and outside the small one
					if (d1 <= 2.8f && d2 > 2.4f) {
						setPixel(f, x, y, { 255, 235, 170 });
					}
				}
			}
		}

		// Move the sparkles and spawn new ones behind the sleigh
		void updateTrail(float dt, int sx, int sleighY) {
			// Age the live sparkles and let them sink slowly
			for (auto &p : trail) {
				if (p.life <= 0) {
					continue;
				}
				p.life -= dt;
				p.y += dt * 1.5f;
			}

			// Spawn 18 sparkles per second
			spawnAccumulator += dt * 18.0f;
			while (spawnAccumulator >= 1.0f) {
				spawnAccumulator -= 1.0f;
				for (auto &p : trail) {
					if (p.life <= 0) {
						// Put the sparkle at the back of the sleigh with a random life
						p = { sx + frand(-2.0f, 1.0f), sleighY + frand(5.0f, 9.0f), frand(0.6f, 1.4f) };
						break;
					}
				}
			}
		}

		// Draw the sparkles, white when new and gold and dim as they die
		void drawTrail(Frame &f) {
			for (const auto &p : trail) {
				if (p.life <= 0) {
					continue;
				}

				// Blend from gold to white by the remaining life
				Color c = blendColor(GOLD, WHITE, p.life);

				// Draw the sparkle, dimmer as it dies
				addPixel(f, (int)p.x, (int)p.y, scaleColor(c, min(1.0f, p.life)));
			}
		}
	};

	// Define the sprite arrays outside the class (needed before C++17)
	constexpr const char *SleighScene::SLEIGH[];
	constexpr const char *SleighScene::DEER_BODY[];
	constexpr const char *SleighScene::LEGS_A[];
	constexpr const char *SleighScene::LEGS_B[];

} // namespace

// Get the scene (created on first use)
Scene &getSleighScene() {
	static SleighScene scene;
	return scene;
}