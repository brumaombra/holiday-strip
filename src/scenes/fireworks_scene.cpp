#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"
#include "font/font.h"
#include "calendar/countdown.h"

namespace {

	// Fireworks bursting over the year number, for New Year's Eve
	class FireworksScene : public Scene {
	public:
		// Get the name of the scene
		const char *name() const override { return "Fireworks"; }

		// Clear the sky and reset the rockets and sparks
		void begin() override {
			// Clear the canvas
			fillFrame(canvas, {0, 0, 0});

			// Stop all the rockets
			for (auto &r : rockets) {
				r.active = false;
			}

			// Kill all the sparks
			for (auto &p : particles) {
				p.life = 0;
			}

			// Launch the first rocket right away
			nextLaunch = 0;

			// Read the year to show
			year = upcomingYear();
		}

		// Draw the year with the rocket and spark trails on top
		void draw(Frame &f, float t, float dt) override {
			// Fade the trails of the previous frame (the canvas keeps them between frames)
			for (auto &row : canvas) {
				for (auto &p : row) {
					p = scaleColor(p, 0.72f);
				}
			}

			// Launch a new rocket when the timer runs out
			nextLaunch -= dt;
			if (nextLaunch <= 0) {
				launch();
				nextLaunch = frand(0.4f, 1.1f); // Seconds between two rockets
			}

			// Move the rockets and the sparks
			updateRockets(dt);
			updateParticles(dt);

			// Fill the sky with a very dark blue
			fillFrame(f, {0, 0, 6});

			// Draw the year in dim gold when it's known
			if (year > 0) {
				// Center the year on the matrix
				String text = String(year);
				int x0 = (WIDTH - textWidth(text) + 1) / 2;

				// Draw each digit with a slow shimmer running through it
				for (size_t i = 0; i < text.length(); i++) {
					drawChar(f, text[i], x0 + i * GLYPH_ADVANCE, 4,
					         [&](int px, int py) { return scaleColor(GOLD, 0.35f + 0.1f * sinf(t * 2 + px * 0.3f)); });
				}
			}

			// Add the canvas on top of the sky
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					addPixel(f, x, y, canvas[y][x]);
				}
			}
		}

	private:
		// A rocket flying up to its burst height
		struct Rocket {
			float x, y, vy, burstY; // Position, vertical speed (negative is up) and the row where it bursts
			bool active;            // Whether the rocket is flying
		};

		// A spark of a burst
		struct Particle {
			float x, y, vx, vy, life, maxLife; // Position, speed, and remaining and total life in seconds
			Color color;
		};
		static constexpr int MAX_ROCKETS = 4, MAX_PARTICLES = 160; // Sizes of the rocket and spark pools
		Rocket rockets[MAX_ROCKETS];
		Particle particles[MAX_PARTICLES];
		Frame canvas;    // Picture of the trails, faded a bit every frame
		float nextLaunch = 0; // Seconds until the next rocket
		int year = 0;         // Year shown over the fireworks (0 when unknown)

		// Read the year to show over the fireworks (0 before the clock is set)
		static int upcomingYear() {
			// Get the local time (without waiting for the clock to sync)
			struct tm now;
			if (!getLocalTime(&now, 0)) {
				return 0;
			}

			// Return the year that is coming
			return getUpcomingYear(now);
		}

		// Start a rocket at the bottom edge in the first free slot
		void launch() {
			for (auto &r : rockets) {
				// Skip the rockets already flying
				if (r.active) {
					continue;
				}

				// Start the rocket at a random column, flying up at 12 to 16 rows per second to a random burst height
				r = {frand(4, WIDTH - 4), (float)HEIGHT, frand(-16.0f, -12.0f), frand(1.5f, 4.5f), true};
				return; // Launch only one rocket
			}
		}

		// Move the rockets up and burst them at their height
		void updateRockets(float dt) {
			for (auto &r : rockets) {
				// Skip the rockets that aren't flying
				if (!r.active) {
					continue;
				}

				// Move the rocket up
				r.y += r.vy * dt;

				// Leave a glowing dot on the canvas (it fades into a trail)
				addPixel(canvas, (int)r.x, (int)r.y, {255, 180, 80});

				// Burst the rocket when it reaches its height
				if (r.y <= r.burstY) {
					r.active = false;
					burst(r.x, r.y);
				}
			}
		}

		// Explode a rocket into a ring of sparks and a white flash
		void burst(float x, float y) {
			// Pick two random bright colors (every third spark uses the second one)
			Color color = hsvColor(esp_random() % 256, 230, 255);
			Color second = hsvColor(esp_random() % 256, 200, 255);

			// Take the free spark slots for the burst
			int count = 0;
			for (auto &p : particles) {
				// Skip the sparks that are still alive
				if (p.life > 0) {
					continue;
				}

				// Pick a random direction, speed and life
				float angle = frand(0, 2 * PI);
				float speed = frand(4.0f, 11.0f);
				float life = frand(0.8f, 1.6f);
				// Squash vertically: the matrix is much wider than it is tall
				p = {x, y, cosf(angle) * speed, sinf(angle) * speed * 0.6f, life, life, count % 3 ? color : second};

				// Stop once the burst has 36 sparks
				if (++count >= 36) {
					break;
				}
			}

			// Flash a white 3 by 3 square at the burst point
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					addPixel(canvas, (int)x + dx, (int)y + dy, WHITE);
				}
			}
		}

		// Move the sparks with drag and gravity, and draw them fading out
		void updateParticles(float dt) {
			for (auto &p : particles) {
				// Skip the dead sparks
				if (p.life <= 0) {
					continue;
				}

				// Age the spark
				p.life -= dt;

				// Slow the spark down with drag and pull it down with gravity
				p.vx *= 1.0f - 1.8f * dt;
				p.vy = p.vy * (1.0f - 1.8f * dt) + 5.0f * dt;

				// Move the spark
				p.x += p.vx * dt;
				p.y += p.vy * dt;

				// Get the remaining life as a fraction (1 when new)
				float k = p.life / p.maxLife;
				// Sparkle as they fade out
				if (k < 0.35f && esp_random() % 3 == 0) {
					continue;
				}

				// Draw the spark, white-hot when new and dimmer as it dies
				addPixel(canvas, (int)p.x, (int)p.y, scaleColor(blendColor(p.color, WHITE, k > 0.85f ? 0.6f : 0), k));
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getFireworksScene() {
	static FireworksScene scene;
	return scene;
}