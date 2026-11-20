#include "settings_rules.h"

#include <ctype.h>
#include <string.h>

#include "config/config.h"

namespace {
	// The codes of the languages, in the order of Language
	const char *const LANGUAGE_CODES[LANGUAGE_COUNT] = { "it", "en" };

	// The default greeting of each holiday, in each language (Italian, English)
	const char *const DEFAULT_MESSAGES[HOLIDAY_COUNT][LANGUAGE_COUNT] = {
		{ "BUON NATALE!", "MERRY CHRISTMAS!" },
		{ "BUON ANNO!", "HAPPY NEW YEAR!" },
		{ "CIAO!", "HELLO!" },
		{ "BUONA PASQUA!", "HAPPY EASTER!" }
	};

	// The two-language greetings, the defaults before the languages (still saved on the boards that never changed them)
	const char *const LEGACY_MESSAGES[HOLIDAY_COUNT] = { "BUON NATALE * MERRY CHRISTMAS", "BUON ANNO * HAPPY NEW YEAR", "CIAO! * HELLO!", nullptr }; // Easter came after the languages
}

// Keep a valid scene index, or -1 (rotate through all the scenes)
int sanitizeScene(long scene, int sceneCount) {
	return (scene >= 0 && scene < sceneCount) ? (int)scene : -1;
}

// Keep a valid holiday index, or -1 (follow the calendar)
int sanitizeHoliday(long holiday, int holidayCount) {
	return (holiday >= 0 && holiday < holidayCount) ? (int)holiday : -1;
}

// Keep a valid pair of holiday and scene: the scene within its holiday, and no chosen scene while the calendar picks the holiday
void sanitizeHolidayScene(long holiday, long scene, const int *sceneCounts, int holidayCount, int &holidayOutput, int &sceneOutput) {
	holidayOutput = sanitizeHoliday(holiday, holidayCount);
	sceneOutput = holidayOutput >= 0 ? sanitizeScene(scene, sceneCounts[holidayOutput]) : -1;
}

// Keep a valid color of the custom message, or the default one
uint8_t sanitizeMessageColor(long color) {
	return (color >= 0 && color < config::MESSAGE_COLOR_COUNT) ? (uint8_t)color : config::DEFAULT_MESSAGE_COLOR;
}

// Find a language by its code ("it", "en"), or -1 when it's unknown
int findLanguage(const char *code) {
	for (int i = 0; i < LANGUAGE_COUNT; i++) {
		if (strcmp(code, LANGUAGE_CODES[i]) == 0) {
			return i;
		}
	}
	return -1;
}

// Get the code of a language ("it", "en")
const char *getLanguageCode(uint8_t language) {
	return LANGUAGE_CODES[sanitizeLanguage(language)];
}

// Keep a valid language, or the default one
uint8_t sanitizeLanguage(long language) {
	return (language >= 0 && language < LANGUAGE_COUNT) ? (uint8_t)language : DEFAULT_LANGUAGE;
}

// Get the default greeting of a holiday in a language
const char *getDefaultMessage(int holiday, uint8_t language) {
	return DEFAULT_MESSAGES[holiday][sanitizeLanguage(language)];
}

// Check whether a greeting is still a default one (in any language, or the two-language one from before the languages)
bool isDefaultMessage(int holiday, const char *message) {
	if (LEGACY_MESSAGES[holiday] != nullptr && strcmp(message, LEGACY_MESSAGES[holiday]) == 0) {
		return true;
	}
	for (const char *defaultMessage : DEFAULT_MESSAGES[holiday]) {
		if (strcmp(message, defaultMessage) == 0) {
			return true;
		}
	}
	return false;
}

// Keep the power limit within the allowed range
uint32_t sanitizeMaxMilliamps(long maxMilliamps) {
	if (maxMilliamps < (long)config::MIN_MAX_MILLIAMPS) {
		return config::MIN_MAX_MILLIAMPS;
	}
	if (maxMilliamps > (long)config::MAX_MAX_MILLIAMPS) {
		return config::MAX_MAX_MILLIAMPS;
	}
	return (uint32_t)maxMilliamps;
}

// Copy the message in capitals (the font has no lower case), cut to the longest message and to the output
void sanitizeMessage(const char *message, char *output, size_t outputSize) {
	if (outputSize == 0) {
		return;
	}
	size_t length = 0;
	while (message[length] != '\0' && length < config::MAX_MESSAGE_LENGTH && length + 1 < outputSize) {
		output[length] = (char)toupper((unsigned char)message[length]);
		length++;
	}
	output[length] = '\0';
}