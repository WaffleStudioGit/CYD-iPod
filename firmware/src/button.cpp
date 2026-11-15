#include "button.h"
#include "config.h"

void MultiClickButton::begin(int pin, bool activeLow) {
  pin_ = pin;
  activeLow_ = activeLow;
  // IO35 has no internal pull-up — external 10k to 3V3 required.
  pinMode(pin_, INPUT);
  lastRaw_ = readPressed();
  stablePressed_ = lastRaw_;
  lastChangeMs_ = millis();
}

bool MultiClickButton::readPressed() const {
  const bool high = digitalRead(pin_) == HIGH;
  return activeLow_ ? !high : high;
}

void MultiClickButton::update() {
  const uint32_t now = millis();
  const bool raw = readPressed();

  if (raw != lastRaw_) {
    lastRaw_ = raw;
    lastChangeMs_ = now;
  }

  if ((now - lastChangeMs_) >= kBtnDebounceMs && raw != stablePressed_) {
    stablePressed_ = raw;
    if (!stablePressed_) {
      // released → count a click
      if (clickCount_ == 0) {
        firstClickMs_ = now;
      }
      clickCount_++;
    }
  }

  if (clickCount_ > 0 && (now - firstClickMs_) >= kBtnMultiClickWindowMs) {
    switch (clickCount_) {
      case 1: pending_ = ButtonAction::Single; break;
      case 2: pending_ = ButtonAction::Double; break;
      case 3: pending_ = ButtonAction::Triple; break;
      default: pending_ = ButtonAction::Quad; break;
    }
    clickCount_ = 0;
  }
}

ButtonAction MultiClickButton::consume() {
  const ButtonAction a = pending_;
  pending_ = ButtonAction::None;
  return a;
}
