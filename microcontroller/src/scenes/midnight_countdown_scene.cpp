#include "scene_list.h"

#include <time.h>

#include "calendar/countdown.h"
#include "calendar/holiday_calendar.h"
#include "font/font.h"
#include "scenes/effects.h"

namespace {

	// The countdown to midnight on New Year's Eve: the time left on December 31, the last ten seconds one by one, then the new year bursting in (the rest of the time, the coming year)
	class MidnightCountdownScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "countdown"; }
		const char *name() const override { return "Countdown"; }

		// Start counting from the current second
		void begin() override {
			lastSecondsLeft = -2;
			secondStart = 0;
			for (auto &spark : sparks) {
				spark.life = 0;
			}
		}

		// Draw the part of the countdown the clock is at
		void draw(Frame &f, float t, float dt) override {
			// Read the clock (a date before it's set when unknown)
			struct tm now = {};
			if (!getLocalTime(&now, 0)) {
				now = {};
			}
			const long secondsLeft = getSecondsToNewYear(now);

			// Note when a new second starts, for the effects that run within each second
			if (secondsLeft != lastSecondsLeft) {
				lastSecondsLeft = secondsLeft;
				secondStart = t;
			}
			const float secondTime = t - secondStart;

			// Draw the stars behind everything
			fillFrame(f, { 0, 0, 4 });
			stars.draw(f, t);

			// Pick what to show from the time left
			if (isNewYearsFirstMinutes(now)) {
				drawCelebration(f, t, dt, getUpcomingYear(now), now.tm_min == 0 && now.tm_sec == 0 ? secondTime : -1);
			} else if (secondsLeft > 0 && secondsLeft <= FINAL_SECONDS) {
				drawFinalSecond(f, secondsLeft, secondTime);
			} else if (secondsLeft > 0 && secondsLeft <= SECONDS_PER_DAY) {
				drawTimeLeft(f, t, secondsLeft);
			} else {
				drawYear(f, t, getUpcomingYear(now));
			}
		}

		// Place the stars once
		MidnightCountdownScene() { stars.begin(); }

	private:
		static constexpr long FINAL_SECONDS = 10; // The last seconds shown one by one
		static constexpr long SECONDS_PER_DAY = 86400; // The time left is shown on December 31
		static constexpr int CELEBRATION_MINUTES = 2; // The new year bursts in for the first minutes of January 1
		static constexpr int MAX_SPARKS = 90; // Size of the spark pool of the celebration

		// A spark of the celebration
		struct Spark {
			float x, y, vx, vy, life;
			Color color;
		};

		Stars stars;
		Spark sparks[MAX_SPARKS] = {};
		long lastSecondsLeft = -2; // The time left at the previous frame
		float secondStart = 0; // Scene time when the current second started
		float nextBurst = 0; // Seconds until the next burst of the celebration

		// Check whether it's the first minutes of January 1
		static bool isNewYearsFirstMinutes(const struct tm &now) {
			return isClockSet(now) && now.tm_mon == 0 && now.tm_mday == 1 && now.tm_hour == 0 && now.tm_min < CELEBRATION_MINUTES;
		}

		// Draw a text centered on the matrix, colored per pixel
		template <typename ColorFn>
		static void drawCenteredText(Frame &f, const String &text, ColorFn colorAt) {
			const int x0 = (WIDTH - textWidth(text) + 1) / 2;
			for (size_t i = 0; i < text.length(); i++) {
				drawChar(f, text[i], x0 + i * GLYPH_ADVANCE, 3, colorAt);
			}
		}

		// Draw the time left on December 31: hours, minutes and seconds, then minutes and seconds in the last hour, then the seconds
		void drawTimeLeft(Frame &f, float t, long secondsLeft) {
			// Write the time left in the shortest form
			char text[12];
			const long hours = secondsLeft / 3600, minutes = secondsLeft / 60 % 60, seconds = secondsLeft % 60;
			if (hours > 0) {
				snprintf(text, sizeof(text), "%ld:%02ld:%02ld", hours, minutes, seconds);
			} else if (minutes > 0) {
				snprintf(text, sizeof(text), "%ld:%02ld", minutes, seconds);
			} else {
				snprintf(text, sizeof(text), "%ld", seconds);
			}

			// Draw it in gold, warmer and faster pulsing as midnight gets closer
			const float urgency = 1.0f - min(1.0f, secondsLeft / 3600.0f);
			const float pulse = 0.75f + 0.25f * sinf(t * (2.0f + urgency * 8.0f));
			const Color color = blendColor(GOLD, { 255, 70, 20 }, urgency * 0.6f);
			drawCenteredText(f, String(text), [&](int px, int py) { return scaleColor(color, pulse); });
		}

		// Draw one of the last ten seconds: the number, flashing at the start of the second, inside a ring that grows through it
		void drawFinalSecond(Frame &f, long secondsLeft, float secondTime) {
			// The ring around the number, growing and fading over the second
			const float radius = 4.0f + secondTime * 26.0f;
			const Color ringColor = scaleColor(hsvColor((uint8_t)(secondsLeft * 25), 200, 255), max(0.0f, 1.0f - secondTime));
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					// Squash the ring vertically: the matrix is much wider than it is tall
					const float distance = sqrtf(sq(x - WIDTH / 2.0f) + sq((y - HEIGHT / 2.0f) * 3.0f));
					if (fabsf(distance - radius) < 1.2f) {
						addPixel(f, x, y, ringColor);
					}
				}
			}

			// The number, white at the start of the second and cooling to red
			const Color color = blendColor(WHITE, { 255, 40, 30 }, min(1.0f, secondTime * 2.0f));
			drawCenteredText(f, String(secondsLeft), [&](int px, int py) { return color; });
		}

		// Draw the coming year with a glint sweeping across it
		void drawYear(Frame &f, float t, int year) {
			// Show the year when the clock is set, or dots while it isn't
			const String text = year > 0 ? String(year) : String("...");

			// Sweep a white glint across the gold every 3 seconds
			const float glint = fmodf(t, 3.0f) / 3.0f * (WIDTH + 20) - 10;
			drawCenteredText(f, text, [&](int px, int py) {
				const float closeness = max(0.0f, 1.0f - fabsf(px - glint) / 4.0f);
				return blendColor(scaleColor(GOLD, 0.6f), WHITE, closeness);
			});
		}

		// Draw the new year bursting in: the year changing colors, a white flash in its first second (firstSecondTime, -1 after it) and bursts of sparks all over
		void drawCelebration(Frame &f, float t, float dt, int year, float firstSecondTime) {
			// Burst sparks from random spots
			nextBurst -= dt;
			if (nextBurst <= 0) {
				burst(frand(6, WIDTH - 6), frand(2, HEIGHT - 2));
				nextBurst = frand(0.15f, 0.45f); // Seconds between bursts
			}

			// Move and draw the sparks
			for (auto &spark : sparks) {
				if (spark.life <= 0) {
					continue;
				}
				spark.life -= dt;
				spark.vy += 6.0f * dt; // Gravity
				spark.x += spark.vx * dt;
				spark.y += spark.vy * dt;
				addPixel(f, (int)spark.x, (int)spark.y, scaleColor(spark.color, min(1.0f, spark.life * 1.5f)));
			}

			// Draw the year on top, every digit cycling through the colors
			const String text = year > 0 ? String(year) : String("AUGURI!");
			drawCenteredText(f, text, [&](int px, int py) { return hsvColor((uint8_t)(px * 6 + t * 120), 180, 255); });

			// Flash white for the first half second of the year
			const float flash = firstSecondTime < 0 ? 0 : 1.0f - firstSecondTime * 2.0f;
			if (flash <= 0) {
				return;
			}
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = blendColor(f[y][x], WHITE, flash * 0.8f);
				}
			}
		}

		// Explode a burst of sparks at a point
		void burst(float x, float y) {
			const Color color = hsvColor(esp_random() % 256, 220, 255);
			int count = 0;
			for (auto &spark : sparks) {
				if (spark.life > 0) {
					continue;
				}
				const float angle = frand(0, 2 * PI);
				const float speed = frand(3.0f, 9.0f);
				spark = { x, y, cosf(angle) * speed, sinf(angle) * speed * 0.6f, frand(0.5f, 1.0f), color };
				if (++count >= 18) {
					break;
				}
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getMidnightCountdownScene() {
	static MidnightCountdownScene scene;
	return scene;
}