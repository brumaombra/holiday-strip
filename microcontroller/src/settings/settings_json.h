#ifndef SETTINGS_JSON_H
#define SETTINGS_JSON_H

#include <ArduinoJson.h>

#include "settings/settings.h"

// Mapping between the settings and the web API JSON, free of hardware headers so it can be tested on the host

// Write the settings into a JSON object
void writeSettingsToJson(JsonObject json, const Settings &settings);

// Apply the fields of a JSON object to the settings, all or nothing (returns the name of the first invalid field, or nullptr)
// sceneCounts has one entry per holiday; activeHoliday is the holiday playing now, which a chosen scene pins when the calendar picks the holiday
const char *applySettingsFromJson(JsonObjectConst json, Settings &settings, const int *sceneCounts, int activeHoliday);

#endif