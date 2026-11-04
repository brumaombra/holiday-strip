#pragma once

#include <Arduino.h>

#include "display/frame.h"

void setupDisplay();
void showFrame(const Frame &frame, uint8_t brightness, uint32_t maxMilliamps);
uint32_t getEstimatedMilliamps();