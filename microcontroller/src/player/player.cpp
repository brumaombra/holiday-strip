#include "player.h"

#include "config/config.h"
#include "display/display.h"
#include "scenes/scenes.h"

namespace {
	// The frames of the current scene, the next one (during a crossfade), and the blend sent to the matrix
	Frame currentFrame, nextFrame, outputFrame;

	// Rotation state
	int currentScene = 0;
	int nextScene = -1; // -1 when no crossfade is running
	uint32_t sceneStartMs = 0;
	uint32_t fadeStartMs = 0;
	uint32_t lastFrameMs = 0;

	// Show a scene right away
	void startScene(int index) {
		currentScene = index;
		nextScene = -1;
		getScene(index)->begin();
		sceneStartMs = millis();
	}

	// Start a crossfade to another scene (ignored if it is already showing, or a crossfade is running)
	void startFade(int index) {
		if (index == currentScene || nextScene >= 0) {
			return;
		}
		nextScene = index;
		getScene(index)->begin();
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

		// Move to the chosen scene, or to the next one when the rotation is due
		const float sceneTime = (now - sceneStartMs) / 1000.0f;
		if (settings.scene >= 0 && settings.scene != currentScene) {
			startFade(settings.scene);
		}
		if (settings.scene < 0 && sceneTime >= config::SCENE_SECONDS) {
			startFade((currentScene + 1) % getSceneCount());
		}

		// Draw the current scene, and send it when no crossfade is running
		getScene(currentScene)->draw(currentFrame, sceneTime, dt);
		if (nextScene < 0) {
			memcpy(outputFrame, currentFrame, sizeof(Frame));
			showFrame(outputFrame, settings.brightness, settings.maxMilliamps);
			return;
		}

		// Crossfade: blend the next scene over the current one
		const float fadeTime = (now - fadeStartMs) / 1000.0f;
		getScene(nextScene)->draw(nextFrame, fadeTime, dt);
		const float amount = fadeTime / config::FADE_SECONDS;
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				outputFrame[y][x] = blendColor(currentFrame[y][x], nextFrame[y][x], amount);
			}
		}
		showFrame(outputFrame, settings.brightness, settings.maxMilliamps);

		// End of the crossfade: the next scene becomes the current one
		if (amount >= 1.0f) {
			currentScene = nextScene;
			nextScene = -1;
			sceneStartMs = fadeStartMs;
		}
	}
}

// Start with the chosen scene, or the first one of the rotation
void setupPlayer(const Settings &settings) {
	startScene(settings.scene >= 0 ? settings.scene : 0);
	lastFrameMs = millis();
}

// Draw the next frame when it is due
void updatePlayer(const Settings &settings) {
	if (millis() - lastFrameMs >= config::FRAME_MS) {
		renderFrame(settings);
	}
}

// Get the scene on the matrix
int getCurrentSceneIndex() {
	return currentScene;
}

// Get the last frame sent to the matrix (before gamma and brightness)
const Frame &getOutputFrame() {
	return outputFrame;
}