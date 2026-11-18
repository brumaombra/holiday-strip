#ifndef SCENE_LIST_H
#define SCENE_LIST_H

#include "calendar/holiday_calendar.h"
#include "scenes/scene.h"

// Every scene, created on first use

// Christmas
Scene &getGreetingScene();
Scene &getSnowScene();
Scene &getTreesScene();
Scene &getSleighScene();
Scene &getFireplaceScene();
Scene &getGarlandScene();
Scene &getGiftsScene();
Scene &getCandyCaneScene();

// New Year's Eve
Scene &getMidnightCountdownScene();
Scene &getConfettiScene();
Scene &getFireworksScene();
Scene &getChampagneScene();
Scene &getSparklersScene();

// Custom message
Scene &getCustomMessageScene();

// Settings of the greeting scenes
const String &getHolidayMessage(int holiday);
bool isCountdownEnabled();
uint8_t getMessageColor();

#endif