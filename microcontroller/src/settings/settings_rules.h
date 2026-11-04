#ifndef SETTINGS_RULES_H
#define SETTINGS_RULES_H

#include <stddef.h>
#include <stdint.h>

// Limits of the settings, kept free of the hardware so the native tests can check them

// Keep a valid scene index, or -1 (rotate through all the scenes)
int sanitizeScene(long scene, int sceneCount);

// Keep the power limit within the allowed range
uint32_t sanitizeMaxMilliamps(long maxMilliamps);

// Copy the message in capitals (the font has no lower case), cut to the longest message and to the output
void sanitizeMessage(const char *message, char *output, size_t outputSize);

#endif