#pragma once

#include <Arduino.h>

struct BatteryStatus {
  bool senseEnabled;
  bool valid;
  uint16_t millivolts;  // pack voltage estimate
  uint8_t percent;      // 0–100
  const char* label;    // short UI string
};

class BatteryMonitor {
 public:
  void begin();
  BatteryStatus read();

 private:
  BatteryStatus last_{};
};
