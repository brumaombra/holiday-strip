#ifndef SCENES_H
#define SCENES_H

#include <Arduino.h>

#include "scenes/scene.h"

int getSceneCount();
Scene *getScene(int index);
void setGreetingMessage(const String &message);
void setCountdownEnabled(bool enabled);

#endif