#include "scenes.h"

#include "config/config.h"
#include "scenes/scene_list.h"
#include "settings/settings_rules.h"

namespace {
	// The greeting of each holiday
	String holidayMessages[HOLIDAY_COUNT] = { getDefaultMessage(CHRISTMAS, DEFAULT_LANGUAGE), getDefaultMessage(NEW_YEAR, DEFAULT_LANGUAGE), getDefaultMessage(CUSTOM, DEFAULT_LANGUAGE), getDefaultMessage(EASTER, DEFAULT_LANGUAGE) };
	uint8_t matrixLanguage = DEFAULT_LANGUAGE;
	uint8_t messageColor = config::DEFAULT_MESSAGE_COLOR;

	// The scenes of each holiday, in the order they rotate
	Scene *const CHRISTMAS_SCENES[] = { &getGreetingScene(), &getSnowScene(), &getTreesScene(), &getSleighScene(), &getFireplaceScene(), &getGarlandScene(), &getGiftsScene(), &getCandyCaneScene(), &getChristmasCountdownScene() }; // The countdown last, so the scenes saved before it keep their place
	Scene *const NEW_YEAR_SCENES[] = { &getMidnightCountdownScene(), &getConfettiScene(), &getFireworksScene(), &getChampagneScene(), &getSparklersScene() };
	Scene *const CUSTOM_SCENES[] = { &getCustomMessageScene() };
	Scene *const EASTER_SCENES[] = { &getEasterGreetingScene(), &getBunnyScene(), &getPaintedEggsScene(), &getChickScene(), &getCrossesScene(), &getEmptyTombScene(), &getDoveScene(), &getEasterBellsScene() }; // Spring first, then the religious scenes

	// The holidays, in the order of HolidayIndex
	const Holiday HOLIDAYS[HOLIDAY_COUNT] = {
		{ "christmas", "Christmas", CHRISTMAS_SCENES, sizeof(CHRISTMAS_SCENES) / sizeof(CHRISTMAS_SCENES[0]) },
		{ "new-year", "New Year's Eve", NEW_YEAR_SCENES, sizeof(NEW_YEAR_SCENES) / sizeof(NEW_YEAR_SCENES[0]) },
		{ "custom", "Custom message", CUSTOM_SCENES, sizeof(CUSTOM_SCENES) / sizeof(CUSTOM_SCENES[0]) },
		{ "easter", "Easter", EASTER_SCENES, sizeof(EASTER_SCENES) / sizeof(EASTER_SCENES[0]) }
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

// Set the greeting of a holiday
void setHolidayMessage(int holiday, const char *message) {
	holidayMessages[holiday] = message;
}

// Set the color of the custom message (0 to MESSAGE_COLOR_COUNT - 1)
void setMessageColor(uint8_t color) {
	messageColor = color;
}

// Set the language of the texts on the matrix (Language)
void setMatrixLanguage(uint8_t language) {
	matrixLanguage = language;
}

// Get the language of the texts on the matrix
uint8_t getMatrixLanguage() {
	return matrixLanguage;
}

// Get the color of the custom message
uint8_t getMessageColor() {
	return messageColor;
}

// Get the greeting of a holiday
const String &getHolidayMessage(int holiday) {
	return holidayMessages[holiday];
}