#pragma once

#include "scenes/scene.h"

// Every scene, created on first use
Scene &getSnowScene();
Scene &getTreesScene();
Scene &getGreetingScene();
Scene &getCandyCaneScene();
Scene &getSleighScene();
Scene &getGarlandScene();
Scene &getFireplaceScene();
Scene &getGiftsScene();
Scene &getFireworksScene();

// Settings of the greeting scene
const String &getGreetingMessage();
bool isCountdownEnabled();