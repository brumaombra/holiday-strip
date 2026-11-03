#pragma once

#include <Arduino.h>

#include "display.h"

class Scene {
 public:
  virtual ~Scene() = default;
  virtual const char *name() const = 0;
  virtual void begin() {}
  // t: seconds since the scene started, dt: seconds since the previous frame
  virtual void draw(Frame &f, float t, float dt) = 0;
};

namespace scenes {
int count();
Scene *get(int index);
// Text shown by the greeting scene; the countdown is appended when enabled
void setMessage(const String &message);
void setCountdown(bool enabled);
}  // namespace scenes
