#ifndef SCENE_H
#define SCENE_H

#include <Arduino.h>

#include "display/frame.h"

// A scene of the matrix: it draws one frame at a time
class Scene {
public:
	virtual ~Scene() = default;
	virtual const char *name() const = 0;
	virtual void begin() {}
	// Draw a frame: t is the time since the scene started, dt the time since the previous frame (seconds)
	virtual void draw(Frame &f, float t, float dt) = 0;
};

#endif