// Render a scene of the matrix to raw video frames, on the computer, with the real scene code (for previews and videos)
// Built and run by render.sh: render HOLIDAY SCENE SECONDS [YYYY-MM-DDTHH:MM:SS], raw RGB frames on stdout
#include <Arduino.h>

#include <cstdio>
#include <vector>

#include "config/config.h"
#include "scenes/scenes.h"

// The clock of the stubbed getLocalTime: the date the video starts at, and how long it has played
time_t rendererClockStart = 0;
float rendererElapsedSeconds = 0;

namespace {
	constexpr int FPS = 30; // About the 33 ms frames of the firmware
	constexpr int PIXEL = 20; // Video pixels per LED
	constexpr float LED_RADIUS = 7.5f; // Radius of each LED dot, in video pixels

	// Parse a local date and time (YYYY-MM-DDTHH:MM:SS) in the timezone of the matrix
	time_t parseLocalTime(const char *text) {
		struct tm date = {};
		if (sscanf(text, "%d-%d-%dT%d:%d:%d", &date.tm_year, &date.tm_mon, &date.tm_mday, &date.tm_hour, &date.tm_min, &date.tm_sec) != 6) {
			fprintf(stderr, "Bad date: %s\n", text);
			exit(1);
		}
		date.tm_year -= 1900;
		date.tm_mon -= 1;
		date.tm_isdst = -1;
		return mktime(&date);
	}

	// Paint a frame of the matrix as round LEDs on a black board, with a little glow around the lit ones
	void paintFrame(const Frame &frame, std::vector<uint8_t> &image) {
		const int width = WIDTH * PIXEL;
		for (int py = 0; py < HEIGHT * PIXEL; py++) {
			for (int px = 0; px < width; px++) {
				// The LED under this video pixel, and the distance from its center
				const int x = px / PIXEL, y = py / PIXEL;
				const float dx = px % PIXEL - PIXEL / 2.0f + 0.5f, dy = py % PIXEL - PIXEL / 2.0f + 0.5f;
				const float distance = sqrtf(dx * dx + dy * dy);

				// Full color inside the dot, a soft glow just outside it, black between the LEDs
				const float amount = distance <= LED_RADIUS ? 1.0f : max(0.0f, 1.0f - (distance - LED_RADIUS) / 2.5f) * 0.35f;
				const Color c = frame[y][x];
				uint8_t *out = &image[(py * width + px) * 3];
				out[0] = (uint8_t)max(14.0f * (distance <= LED_RADIUS), c.r * amount); // Unlit LEDs show as dark grey dots
				out[1] = (uint8_t)max(14.0f * (distance <= LED_RADIUS), c.g * amount);
				out[2] = (uint8_t)max(16.0f * (distance <= LED_RADIUS), c.b * amount);
			}
		}
	}
}

// Render the frames of one scene to stdout
int main(int argc, char **argv) {
	if (argc < 4) {
		fprintf(stderr, "Usage: render HOLIDAY SCENE SECONDS [YYYY-MM-DDTHH:MM:SS]\n");
		return 1;
	}

	// Use the timezone of the matrix, a fixed random seed, and the clock from the arguments (unknown without a date)
	setenv("TZ", config::TIMEZONE, 1);
	tzset();
	srand(2027);
	if (argc > 4) {
		rendererClockStart = parseLocalTime(argv[4]);
	}

	// Color of the custom message, from MESSAGE_COLOR (0 white to 6 rainbow, like the dashboard)
	if (getenv("MESSAGE_COLOR") != nullptr) {
		setMessageColor((uint8_t)atoi(getenv("MESSAGE_COLOR")));
	}

	// Pick the scene
	const int holidayIndex = atoi(argv[1]), sceneIndex = atoi(argv[2]);
	const float seconds = atof(argv[3]);
	if (holidayIndex < 0 || holidayIndex >= HOLIDAY_COUNT || sceneIndex < 0 || sceneIndex >= getHoliday(holidayIndex).sceneCount) {
		fprintf(stderr, "No such scene\n");
		return 1;
	}
	Scene *scene = getHoliday(holidayIndex).scenes[sceneIndex];
	fprintf(stderr, "Rendering %s: %s, %.1f s\n", getHoliday(holidayIndex).name, scene->name(), seconds);

	// Draw every frame like the player does, and write it out
	static Frame frame;
	std::vector<uint8_t> image(WIDTH * PIXEL * HEIGHT * PIXEL * 3);
	const float dt = 1.0f / FPS;
	scene->begin();
	for (int i = 0; i < (int)(seconds * FPS); i++) {
		rendererElapsedSeconds = i * dt;
		scene->draw(frame, i * dt, dt);
		paintFrame(frame, image);
		fwrite(image.data(), 1, image.size(), stdout);
	}
	return 0;
}