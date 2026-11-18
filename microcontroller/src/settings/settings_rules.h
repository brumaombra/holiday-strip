#ifndef SETTINGS_RULES_H
#define SETTINGS_RULES_H

#include <stddef.h>
#include <stdint.h>

// Limits of the settings, kept free of the hardware so the native tests can check them

// Keep a valid scene index, or -1 (rotate through all the scenes)
int sanitizeScene(long scene, int sceneCount);

// Keep a valid holiday index, or -1 (follow the calendar)
int sanitizeHoliday(long holiday, int holidayCount);

// Keep a valid pair of holiday and scene: the scene within its holiday, and no chosen scene while the calendar picks the holiday (sceneCounts has one entry per holiday)
void sanitizeHolidayScene(long holiday, long scene, const int *sceneCounts, int holidayCount, int &holidayOutput, int &sceneOutput);

// Keep a valid color of the custom message, or the default one
uint8_t sanitizeMessageColor(long color);

// Keep the power limit within the allowed range
uint32_t sanitizeMaxMilliamps(long maxMilliamps);

// Copy the message in capitals (the font has no lower case), cut to the longest message and to the output
void sanitizeMessage(const char *message, char *output, size_t outputSize);

#endif