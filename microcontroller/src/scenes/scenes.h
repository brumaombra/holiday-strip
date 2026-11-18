#ifndef SCENES_H
#define SCENES_H

#include <Arduino.h>

#include "calendar/holiday_calendar.h"
#include "scenes/scene.h"

// A holiday and the scenes it rotates through
struct Holiday {
	const char *id; // Stable name for the web app, which translates it
	const char *name; // English name, for a web app that doesn't know the holiday
	Scene *const *scenes;
	int sceneCount;
};

const Holiday &getHoliday(int index);
void getSceneCounts(int *sceneCounts);
void setHolidayMessage(int holiday, const char *message);
void setCountdownEnabled(bool enabled);

#endif