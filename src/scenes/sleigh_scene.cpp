#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"

namespace {

	// Santa's sleigh and reindeer flying across a starry sky, leaving a trail of sparkles
	class SleighScene : public Scene {
	public:
		const char *name() const override { return "Santa's sleigh"; }

		SleighScene() { stars.begin(); }

		void begin() override {
			for (auto &p : trail) {
				p.life = 0;
			}
		}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, {0, 0, 12});
			stars.draw(f, t);
			drawMoon(f);

			const int total = 38;
			float x0 = -total + fmodf(t * 14.0f, WIDTH + total + 12);
			int sx = (int)x0;
			int sleighY = 1 + (int)lroundf(sinf(t * 3.0f) * 0.7f);
			int deer1Y = 2 + (int)lroundf(sinf(t * 3.0f + 1.0f) * 0.7f);
			int deer2Y = 2 + (int)lroundf(sinf(t * 3.0f + 2.0f) * 0.7f);
			bool legs = ((int)(t * 6)) % 2;

			updateTrail(dt, sx, sleighY);
			drawTrail(f);

			// Harness
			for (int x = sx + 12; x < sx + 16; x++) {
				setPixel(f, x, sleighY + 6, GOLD);
			}
			for (int x = sx + 25; x < sx + 28; x++) {
				setPixel(f, x, deer1Y + 4, GOLD);
			}

			drawSprite(f, SLEIGH, 10, sx, sleighY, t);
			drawDeer(f, sx + 15, deer1Y, legs, false, t);
			drawDeer(f, sx + 28, deer2Y, !legs, true, t);
		}

	private:
		struct Particle {
			float x, y, life;
		};
		static constexpr int MAX_PARTICLES = 40;
		Particle trail[MAX_PARTICLES];
		float spawnAccumulator = 0;
		Stars stars;

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
		static constexpr const char *DEER_BODY[6] = {
		    "......a.a.",
		    ".......a..",
		    "......bbbn",
		    "w.....bb..",
		    "bbbbbbbb..",
		    "bbbbbbb...",
		};
		static constexpr const char *LEGS_A[2] = {".b....b.b.", "b......b.b"};
		static constexpr const char *LEGS_B[2] = {"..b...b...", "..b...b..."};

		void drawDeer(Frame &f, int x, int y, bool legPose, bool rudolph, float t) {
			for (int r = 0; r < 6; r++) {
				for (int c = 0; DEER_BODY[r][c]; c++) {
					char ch = DEER_BODY[r][c];
					if (ch == 'n' && !rudolph) {
						ch = 'b';
					}
					if (ch != '.') {
						setPixel(f, x + c, y + r, spriteColor(ch, t));
					}
				}
			}
			drawSprite(f, legPose ? LEGS_A : LEGS_B, 2, x, y + 6, t);
		}

		static void drawMoon(Frame &f) {
			for (int y = 0; y < 7; y++) {
				for (int x = 60; x < 72; x++) {
					float d1 = hypotf(x - 66.0f, y - 3.0f);
					float d2 = hypotf(x - 67.6f, y - 2.2f);
					if (d1 <= 2.8f && d2 > 2.4f) {
						setPixel(f, x, y, {255, 235, 170});
					}
				}
			}
		}

		void updateTrail(float dt, int sx, int sleighY) {
			for (auto &p : trail) {
				if (p.life <= 0) {
					continue;
				}
				p.life -= dt;
				p.y += dt * 1.5f;
			}
			spawnAccumulator += dt * 18.0f;
			while (spawnAccumulator >= 1.0f) {
				spawnAccumulator -= 1.0f;
				for (auto &p : trail) {
					if (p.life <= 0) {
						p = {sx + frand(-2.0f, 1.0f), sleighY + frand(5.0f, 9.0f), frand(0.6f, 1.4f)};
						break;
					}
				}
			}
		}

		void drawTrail(Frame &f) {
			for (const auto &p : trail) {
				if (p.life <= 0) {
					continue;
				}
				Color c = blendColor(GOLD, WHITE, p.life);
				addPixel(f, (int)p.x, (int)p.y, scaleColor(c, min(1.0f, p.life)));
			}
		}
	};
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