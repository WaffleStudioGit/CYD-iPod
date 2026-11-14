#include "power.h"
#include "config.h"
#include "pins.h"
#include <esp_sleep.h>

void PowerManager::begin(int backlightPin) {
  blPin_ = backlightPin;
  pinMode(blPin_, OUTPUT);
  setDisplayOn(true);
  poke();
}

void PowerManager::poke() {
  lastActivityMs_ = millis();
  if (!displayOn_) {
    setDisplayOn(true);
  }
}

void PowerManager::setDisplayOn(bool on) {
  displayOn_ = on;
  digitalWrite(blPin_, on ? HIGH : LOW);
  if (!on) {
    displayOffAtMs_ = millis();
  }
}

void PowerManager::update() {
  if (idleDisplayMs_ == 0) {
    return;
  }
  const uint32_t now = millis();
  if (displayOn_ && (now - lastActivityMs_) >= idleDisplayMs_) {
    setDisplayOn(false);
  }
  if (!displayOn_ && idleSleepMs_ > 0 &&
      (now - displayOffAtMs_) >= idleSleepMs_) {
    // Wake on button (IO35 ext1) — needs external pull-up so line is high when idle.
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN, 0);  // wake on LOW (press)
    esp_deep_sleep_start();
  }
}
