#pragma once

#include <Arduino.h>

#include "display/frame.h"
#include "settings/settings.h"

void setupPlayer(const Settings &settings);
void updatePlayer(const Settings &settings);
void showBlackFrame(uint32_t maxMilliamps);
int getCurrentSceneIndex();
const Frame &getOutputFrame();