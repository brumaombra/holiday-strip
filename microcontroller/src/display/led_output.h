#ifndef LED_OUTPUT_H
#define LED_OUTPUT_H

#include <Arduino.h>

#include "display/frame.h"

// What the matrix receives: the chain order, gamma and brightness, and the power limit, kept free of the hardware so the native tests can check them

constexpr int LED_COUNT = WIDTH * HEIGHT;
constexpr size_t LED_BYTES = LED_COUNT * 3;

void initLedOutput();
int getPixelIndex(int x, int y);
uint32_t buildLedOutput(const Frame &frame, uint8_t brightness, uint32_t maxMilliamps, uint8_t *output);

#endif