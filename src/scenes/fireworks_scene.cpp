#include "scene_list.h"

#include <time.h>

#include "scenes/effects.h"
#include "font/font.h"
#include "calendar/countdown.h"

namespace {

	// Fireworks bursting over the year number, for New Year's Eve
	class FireworksScene : public Scene {
	public:
		const char *name() const override { return "Fireworks"; }

		void begin() override {
			fillFrame(canvas, {0, 0, 0});
			for (auto &r : rockets) {
				r.active = false;
			}
			for (auto &p : particles) {
				p.life = 0;
			}
			nextLaunch = 0;
			year = upcomingYear();
		}

		void draw(Frame &f, float t, float dt) override {
			for (auto &row : canvas) {
				for (auto &p : row) {
					p = scaleColor(p, 0.72f);
				}
			}

			nextLaunch -= dt;
			if (nextLaunch <= 0) {
				launch();
				nextLaunch = frand(0.4f, 1.1f);
			}
			updateRockets(dt);
			updateParticles(dt);

			fillFrame(f, {0, 0, 6});
			if (year > 0) {
				String text = String(year);
				int x0 = (WIDTH - textWidth(text) + 1) / 2;
				for (size_t i = 0; i < text.length(); i++) {
					drawChar(f, text[i], x0 + i * GLYPH_ADVANCE, 4,
					         [&](int px, int py) { return scaleColor(GOLD, 0.35f + 0.1f * sinf(t * 2 + px * 0.3f)); });
				}
			}
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					addPixel(f, x, y, canvas[y][x]);
				}
			}
		}

	private:
		struct Rocket {
			float x, y, vy, burstY;
			bool active;
		};
		struct Particle {
			float x, y, vx, vy, life, maxLife;
			Color color;
		};
		static constexpr int MAX_ROCKETS = 4, MAX_PARTICLES = 160;
		Rocket rockets[MAX_ROCKETS];
		Particle particles[MAX_PARTICLES];
		Frame canvas;
		float nextLaunch = 0;
		int year = 0;

		// Read the year to show over the fireworks (0 before the clock is set)
		static int upcomingYear() {
			struct tm now;
			if (!getLocalTime(&now, 0)) {
				return 0;
			}
			return getUpcomingYear(now);
		}

		void launch() {
			for (auto &r : rockets) {
				if (r.active) {
					continue;
				}
				r = {frand(4, WIDTH - 4), (float)HEIGHT, frand(-16.0f, -12.0f), frand(1.5f, 4.5f), true};
				return;
			}
		}

		void updateRockets(float dt) {
			for (auto &r : rockets) {
				if (!r.active) {
					continue;
				}
				r.y += r.vy * dt;
				addPixel(canvas, (int)r.x, (int)r.y, {255, 180, 80});
				if (r.y <= r.burstY) {
					r.active = false;
					burst(r.x, r.y);
				}
			}
		}

		void burst(float x, float y) {
			Color color = hsvColor(esp_random() % 256, 230, 255);
			Color second = hsvColor(esp_random() % 256, 200, 255);
			int count = 0;
			for (auto &p : particles) {
				if (p.life > 0) {
					continue;
				}
				float angle = frand(0, 2 * PI);
				float speed = frand(4.0f, 11.0f);
				float life = frand(0.8f, 1.6f);
				// Squash vertically: the matrix is much wider than it is tall
				p = {x, y, cosf(angle) * speed, sinf(angle) * speed * 0.6f, life, life, count % 3 ? color : second};
				if (++count >= 36) {
					break;
				}
			}
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					addPixel(canvas, (int)x + dx, (int)y + dy, WHITE);
				}
			}
		}

		void updateParticles(float dt) {
			for (auto &p : particles) {
				if (p.life <= 0) {
					continue;
				}
				p.life -= dt;
				p.vx *= 1.0f - 1.8f * dt;
				p.vy = p.vy * (1.0f - 1.8f * dt) + 5.0f * dt;
				p.x += p.vx * dt;
				p.y += p.vy * dt;
				float k = p.life / p.maxLife;
				// Sparkle as they fade out
				if (k < 0.35f && esp_random() % 3 == 0) {
					continue;
				}
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