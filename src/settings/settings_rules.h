#pragma once

#include <Arduino.h>

// Limits of the settings received from the web, kept free of the hardware so the native tests can check them

uint8_t sanitizeBrightness(long brightness);
int sanitizeScene(long scene, int sceneCount);
uint32_t sanitizeMaxMilliamps(long maxMilliamps);
void sanitizeMessage(const char *message, char *output, size_t outputSize);