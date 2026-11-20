#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// A white dove carrying an olive branch flies across a soft morning sky, flapping its wings, with clouds drifting behind and sparkles in its wake
	class DoveScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "dove"; }
		const char *name() const override { return "Dove of peace"; }

		// Clear the sparkles and put the clouds at random heights
		void begin() override {
			for (auto &sparkle : trail) {
				sparkle.life = 0;
			}
			for (int i = 0; i < CLOUD_COUNT; i++) {
				clouds[i].x = frand(0, WIDTH + CLOUD_WIDTH);
				clouds[i].y = i % 2 == 0 ? 0 : 9; // Alternate between the top and the bottom, out of the way of the dove
				clouds[i].speed = frand(1.5f, 4.0f);
			}
			spawnAccumulator = 0;
		}

		// Draw the sky, the clouds, the sparkles and the flying dove
		void draw(Frame &f, float t, float dt) override {
			drawSky(f, t);
			drawClouds(f, t);

			// Fly across the matrix from left to right, with a gap before the next pass
			const float x = -SPRITE_WIDTH + fmodf(t * SPEED, WIDTH + SPRITE_WIDTH + 14);
			const int frame = (int)(t * 6.0f) % 4; // Wings up, middle, down, middle

			updateTrail(dt, x, 0);
			drawTrail(f);
			drawDove(f, (int)x, 0, WINGS[frame == 3 ? 1 : frame]);
		}

	private:
		static constexpr float SPEED = 11.0f; // Columns per second
		static constexpr int SPRITE_WIDTH = 22;
		static constexpr int SPRITE_HEIGHT = 12;
		static constexpr int CLOUD_COUNT = 4;
		static constexpr int CLOUD_WIDTH = 14;
		static constexpr int MAX_SPARKLES = 24;

		// A cloud drifting to the right
		struct Cloud {
			float x, speed;
			int y;
		};

		// A sparkle in the wake of the dove
		struct Sparkle {
			float x, y, life; // Position and remaining life in seconds
		};

		Cloud clouds[CLOUD_COUNT] = {};
		Sparkle trail[MAX_SPARKLES] = {};
		float spawnAccumulator = 0; // Fraction of a sparkle waiting to be spawned

		// The body of the dove, drawn under the wings: w white, s shaded white, o beak, e eye, b twig, l leaf
		static constexpr const char *BODY[SPRITE_HEIGHT] = {
			"......................",
			"......................",
			"......................",
			"......................",
			"......................",
			"w.............wwe..ll.",
			"wwwwwwwwwwwwwwwwwobbbb",
			"wwww.ssssssssss....ll.",
			"w.....................",
			"......................",
			"......................",
			"......................",
		};

		// The three positions of the wings, drawn over the body: w white, s shaded white
		static constexpr const char *WINGS[3][SPRITE_HEIGHT] = {
			{
				"......................",
				".......ww.............",
				"......www.............",
				".....wwwww............",
				".....wwwwwww..........",
				"....wwwwwwwww.........",
				".....wwwwwwwws........",
				"......................",
				"......................",
				"......................",
				"......................",
				"......................",
			},
			{
				"......................",
				"......................",
				"......................",
				"......................",
				".....wwwwwww..........",
				"..wwwwwwwwwww.........",
				"...sssssssss..........",
				"......................",
				"......................",
				"......................",
				"......................",
				"......................",
			},
			{
				"......................",
				"......................",
				"......................",
				"......................",
				"......................",
				"......................",
				"......................",
				"....wwwwwwwww.........",
				".....wwwwwwws.........",
				"......wwwwss..........",
				".......wwss...........",
				"........ws............",
			},
		};

		// Get the color of a sprite character
		static Color pixelColor(char c) {
			switch (c) {
				case 'w':
					return { 235, 235, 245 };
				case 's':
					return { 150, 160, 205 };
				case 'o':
					return { 255, 140, 20 };
				case 'e':
					return { 120, 140, 190 };
				case 'b':
					return { 120, 80, 30 };
				case 'l':
					return { 60, 200, 50 };
				default:
					return { 0, 0, 0 };
			}
		}

		// Draw a sprite layer at a position, leaving the '.' pixels alone
		static void drawLayer(Frame &f, const char *const *rows, int x, int y) {
			for (int r = 0; r < SPRITE_HEIGHT; r++) {
				for (int c = 0; rows[r][c]; c++) {
					if (rows[r][c] != '.') {
						setPixel(f, x + c, y + r, pixelColor(rows[r][c]));
					}
				}
			}
		}

		// Draw the dove: its body, then the wings in front
		static void drawDove(Frame &f, int x, int y, const char *const *wings) {
			drawLayer(f, BODY, x, y);
			drawLayer(f, wings, x, y);
		}

		// Fill the sky with a soft gradient and a faint shimmer of light
		static void drawSky(Frame &f, float t) {
			for (int y = 0; y < HEIGHT; y++) {
				const Color c = blendColor({ 3, 10, 45 }, { 25, 55, 115 }, y / (float)(HEIGHT - 1));
				for (int x = 0; x < WIDTH; x++) {
					const float shimmer = 0.5f + 0.5f * sinf(x * 0.25f - y * 0.4f + t * 0.5f);
					f[y][x] = blendColor(c, { 50, 80, 140 }, shimmer * 0.12f);
				}
			}
		}

		// Move the clouds to the right (wrapping around) and draw each as three soft blobs
		void drawClouds(Frame &f, float t) const {
			for (const auto &cloud : clouds) {
				const int x = (int)fmodf(cloud.x + cloud.speed * t, WIDTH + CLOUD_WIDTH) - CLOUD_WIDTH;
				for (int dx = 0; dx < CLOUD_WIDTH; dx++) {
					const bool bump = dx > 3 && dx < 9;
					for (int dy = bump ? -1 : 0; dy < 2; dy++) {
						if (dx == 0 || dx == CLOUD_WIDTH - 1) {
							if (dy == 0) {
								continue;
							}
						}
						setPixel(f, x + dx, cloud.y + dy, dy < 0 ? Color { 60, 85, 135 } : Color { 50, 75, 125 });
					}
				}
			}
		}

		// Move the sparkles and spawn new ones behind the tail of the dove
		void updateTrail(float dt, float x, int y) {
			spawnAccumulator += dt * 9.0f;
			for (auto &sparkle : trail) {
				if (sparkle.life > 0) {
					sparkle.life -= dt;
					continue;
				}
				if (spawnAccumulator >= 1.0f && x > -SPRITE_WIDTH + 4) {
					spawnAccumulator -= 1.0f;
					sparkle = { x + frand(-1, 5), y + frand(2, 9), frand(0.6f, 1.2f) };
				}
			}
			spawnAccumulator = min(spawnAccumulator, 1.0f);
		}

		// Draw the sparkles, fading as they age
		void drawTrail(Frame &f) const {
			for (const auto &sparkle : trail) {
				if (sparkle.life > 0) {
					addPixel(f, (int)sparkle.x, (int)sparkle.y, scaleColor({ 255, 235, 150 }, min(1.0f, sparkle.life * 1.5f) * 0.8f));
				}
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getDoveScene() {
	static DoveScene scene;
	return scene;
}
