#ifndef SETTINGS_RULES_H
#define SETTINGS_RULES_H

#include <stddef.h>
#include <stdint.h>

#include "calendar/holiday_calendar.h"
#include "config/language.h"

// Limits of the settings, kept free of the hardware so the native tests can check them

// Keep a valid scene index, or -1 (rotate through all the scenes)
int sanitizeScene(long scene, int sceneCount);

// Keep a valid holiday index, or -1 (follow the calendar)
int sanitizeHoliday(long holiday, int holidayCount);

// Keep a valid pair of holiday and scene: the scene within its holiday, and no chosen scene while the calendar picks the holiday (sceneCounts has one entry per holiday)
void sanitizeHolidayScene(long holiday, long scene, const int *sceneCounts, int holidayCount, int &holidayOutput, int &sceneOutput);

// Keep a valid color of the custom message, or the default one
uint8_t sanitizeMessageColor(long color);

// Find a language by its code ("it", "en"), or -1 when it's unknown
int findLanguage(const char *code);

// Get the code of a language ("it", "en")
const char *getLanguageCode(uint8_t language);

// Keep a valid language, or the default one
uint8_t sanitizeLanguage(long language);

// Get the default greeting of a holiday in a language
const char *getDefaultMessage(int holiday, uint8_t language);

// Check whether a greeting is still a default one (in any language, or the two-language one from before the languages), so a change of language can translate it
bool isDefaultMessage(int holiday, const char *message);

// Keep the power limit within the allowed range
uint32_t sanitizeMaxMilliamps(long maxMilliamps);

// Copy the message in capitals (the font has no lower case), cut to the longest message and to the output
void sanitizeMessage(const char *message, char *output, size_t outputSize);

#endif