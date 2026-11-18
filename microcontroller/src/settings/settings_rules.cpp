#include "settings_rules.h"

#include <ctype.h>

#include "config/config.h"

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