#pragma once

#include <Arduino.h>

enum class ButtonAction : uint8_t {
  None = 0,
  Single,    // play/pause
  Double,    // next
  Triple,    // previous / restart
  Quad       // cycle audio output mode (local ↔ BT)
};

class MultiClickButton {
 public:
  void begin(int pin, bool activeLow = true);
  void update();
  ButtonAction consume();

 private:
  int pin_ = -1;
  bool activeLow_ = true;
  bool stablePressed_ = false;
  bool lastRaw_ = false;
  uint32_t lastChangeMs_ = 0;
  uint8_t clickCount_ = 0;
  uint32_t firstClickMs_ = 0;
  ButtonAction pending_ = ButtonAction::None;

  bool readPressed() const;
};
