#include "scenes.h"

#include "config/config.h"
#include "scenes/scene_list.h"

namespace {
	// The greeting of each holiday, and the Christmas countdown switch
	String holidayMessages[HOLIDAY_COUNT] = { config::DEFAULT_MESSAGE, config::DEFAULT_NEW_YEAR_MESSAGE };
	bool countdownEnabled = config::DEFAULT_COUNTDOWN;

	// The scenes of each holiday, in the order they rotate
	Scene *const CHRISTMAS_SCENES[] = { &getGreetingScene(), &getSnowScene(), &getTreesScene(), &getSleighScene(), &getFireplaceScene(), &getGarlandScene(), &getGiftsScene(), &getCandyCaneScene() };
	Scene *const NEW_YEAR_SCENES[] = { &getMidnightCountdownScene(), &getConfettiScene(), &getFireworksScene(), &getChampagneScene(), &getSparklersScene() };

	// The holidays, in the order of HolidayIndex
	const Holiday HOLIDAYS[HOLIDAY_COUNT] = {
		{ "christmas", "Christmas", CHRISTMAS_SCENES, sizeof(CHRISTMAS_SCENES) / sizeof(CHRISTMAS_SCENES[0]) },
		{ "new-year", "New Year's Eve", NEW_YEAR_SCENES, sizeof(NEW_YEAR_SCENES) / sizeof(NEW_YEAR_SCENES[0]) }
	};
}

// Get a holiday by its index
const Holiday &getHoliday(int index) {
	return HOLIDAYS[index];
}

// Write the number of scenes of each holiday (sceneCounts has room for HOLIDAY_COUNT entries)
void getSceneCounts(int *sceneCounts) {
	for (int i = 0; i < HOLIDAY_COUNT; i++) {
		sceneCounts[i] = HOLIDAYS[i].sceneCount;
	}
}

// Set the greeting of a holiday (the Christmas countdown is added after the Christmas one when enabled)
void setHolidayMessage(int holiday, const char *message) {
	holidayMessages[holiday] = message;
}

// Turn the Christmas countdown of the greeting on or off
void setCountdownEnabled(bool enabled) {
	countdownEnabled = enabled;
}

// Get the greeting of a holiday
const String &getHolidayMessage(int holiday) {
	return holidayMessages[holiday];
}

// Check whether the Christmas countdown is on
bool isCountdownEnabled() {
	return countdownEnabled;
}