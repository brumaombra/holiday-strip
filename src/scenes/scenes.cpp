#include "scenes.h"

#include "config/config.h"
#include "scenes/scene_list.h"

namespace {
	// Settings of the greeting scene
	String greetingMessage = config::DEFAULT_MESSAGE;
	bool countdownEnabled = config::DEFAULT_COUNTDOWN;

	// The scenes, in the order they rotate
	Scene *const SCENES[] = {&getGreetingScene(), &getSnowScene(), &getTreesScene(), &getSleighScene(), &getFireplaceScene(), &getGarlandScene(), &getGiftsScene(), &getCandyCaneScene(), &getFireworksScene()};
}

// Get the number of scenes
int getSceneCount() {
	return sizeof(SCENES) / sizeof(SCENES[0]);
}

// Get a scene by its position in the rotation
Scene *getScene(int index) {
	return SCENES[index];
}

// Set the text of the greeting (the countdown is added after it when enabled)
void setGreetingMessage(const String &message) {
	greetingMessage = message;
}

// Turn the Christmas countdown of the greeting on or off
void setCountdownEnabled(bool enabled) {
	countdownEnabled = enabled;
}

// Get the text of the greeting
const String &getGreetingMessage() {
	return greetingMessage;
}

// Check whether the Christmas countdown is on
bool isCountdownEnabled() {
	return countdownEnabled;
}