#include "player.h"

#include <time.h>

#include "calendar/holiday_calendar.h"
#include "config/config.h"
#include "display/display.h"
#include "scenes/scenes.h"

namespace {
	// A scene of a holiday
	struct SceneRef {
		int holiday;
		int scene;
	};

	// The New Year's Eve scene shown at midnight
	constexpr int MIDNIGHT_SCENE = 0;

	// The frames of the current scene, the next one (during a crossfade), and the blend sent to the matrix
	Frame currentFrame, nextFrame, outputFrame;

	// Rotation state
	SceneRef current = { CHRISTMAS, 0 };
	SceneRef next = { CHRISTMAS, -1 }; // Scene -1 when no crossfade is running
	uint32_t sceneStartMs = 0;
	uint32_t fadeStartMs = 0;
	uint32_t lastFrameMs = 0;

	// Get a scene by its holiday and position
	Scene *getSceneOf(SceneRef ref) {
		return getHoliday(ref.holiday).scenes[ref.scene];
	}

	// Read the local time (zeros, a date before the clock was set, when it's unknown)
	struct tm readClock() {
		struct tm now = {};
		if (!getLocalTime(&now, 0)) {
			now = {};
		}
		return now;
	}

	// Pick the scene that should play: the chosen one, the countdown at the New Year moment, the first one of a holiday that just started, or the next one when the rotation is due
	SceneRef pickScene(const Settings &settings, const struct tm &now, float sceneTime) {
		const int holiday = getActiveHoliday(settings.holiday, now);
		const int sceneCount = getHoliday(holiday).sceneCount;

		// A chosen scene (it always comes with its holiday)
		if (settings.scene >= 0) {
			return { holiday, min(settings.scene, sceneCount - 1) };
		}

		// The countdown at midnight on New Year's Eve
		if (holiday == NEW_YEAR && isNewYearMoment(now)) {
			return { NEW_YEAR, MIDNIGHT_SCENE };
		}

		// Another holiday starts from its first scene
		if (holiday != current.holiday) {
			return { holiday, 0 };
		}

		// The next scene once the current one has played long enough
		if (sceneTime >= config::SCENE_SECONDS) {
			return { holiday, (current.scene + 1) % sceneCount };
		}
		return current;
	}

	// Show a scene right away
	void startScene(SceneRef ref) {
		current = ref;
		next.scene = -1;
		getSceneOf(ref)->begin();
		sceneStartMs = millis();
	}

	// Start a crossfade to another scene (ignored if it is already showing, or a crossfade is running)
	void startFade(SceneRef ref) {
		if ((ref.holiday == current.holiday && ref.scene == current.scene) || next.scene >= 0) {
			return;
		}
		next = ref;
		getSceneOf(ref)->begin();
		fadeStartMs = millis();
	}

	// Draw and send one frame: the current scene, or the crossfade to the next one
	void renderFrame(const Settings &settings) {
		const uint32_t now = millis();
		const float dt = min(0.1f, (now - lastFrameMs) / 1000.0f);
		lastFrameMs = now;

		// Display off: send a black frame
		if (!settings.on) {
			fillFrame(outputFrame, { 0, 0, 0 });
			showFrame(outputFrame, 0, settings.maxMilliamps);
			return;
		}

		// Move to the scene that should play
		const float sceneTime = (now - sceneStartMs) / 1000.0f;
		startFade(pickScene(settings, readClock(), sceneTime));

		// Draw the current scene, and send it when no crossfade is running
		getSceneOf(current)->draw(currentFrame, sceneTime, dt);
		if (next.scene < 0) {
			memcpy(outputFrame, currentFrame, sizeof(Frame));
			showFrame(outputFrame, settings.brightness, settings.maxMilliamps);
			return;
		}

		// Crossfade: blend the next scene over the current one
		const float fadeTime = (now - fadeStartMs) / 1000.0f;
		getSceneOf(next)->draw(nextFrame, fadeTime, dt);
		const float amount = fadeTime / config::FADE_SECONDS;
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				outputFrame[y][x] = blendColor(currentFrame[y][x], nextFrame[y][x], amount);
			}
		}
		showFrame(outputFrame, settings.brightness, settings.maxMilliamps);

		// End of the crossfade: the next scene becomes the current one
		if (amount >= 1.0f) {
			current = next;
			next.scene = -1;
			sceneStartMs = fadeStartMs;
		}
	}
}

// Start with the scene that should play now
void setupPlayer(const Settings &settings) {
	current = { -1, -1 }; // No holiday yet, so the pick starts the first scene of the active one
	startScene(pickScene(settings, readClock(), 0));
	lastFrameMs = millis();
}

// Draw the next frame when it is due
void updatePlayer(const Settings &settings) {
	if (millis() - lastFrameMs >= config::FRAME_MS) {
		renderFrame(settings);
	}
}

// Get the holiday on the matrix
int getCurrentHolidayIndex() {
	return current.holiday;
}

// Get the scene on the matrix, within its holiday
int getCurrentSceneIndex() {
	return current.scene;
}

// Get the last frame sent to the matrix (before gamma and brightness)
const Frame &getOutputFrame() {
	return outputFrame;
}