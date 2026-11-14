#pragma once

#include <Arduino.h>
#include "config.h"

class PowerManager {
 public:
  void begin(int backlightPin);
  void poke();           // call on any user activity
  void update();         // call from loop
  bool displayOn() const { return displayOn_; }
  void setDisplayOn(bool on);
  void setIdleDisplayMs(uint32_t ms) { idleDisplayMs_ = ms; }
  void setIdleSleepMs(uint32_t ms) { idleSleepMs_ = ms; }
  uint32_t idleDisplayMs() const { return idleDisplayMs_; }

 private:
  int blPin_ = -1;
  bool displayOn_ = true;
  uint32_t lastActivityMs_ = 0;
  uint32_t idleDisplayMs_ = IDLE_DISPLAY_OFF_MS;
  uint32_t idleSleepMs_ = IDLE_DEEP_SLEEP_MS;
  uint32_t displayOffAtMs_ = 0;
};
