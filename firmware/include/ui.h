#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "battery.h"
#include "audio_player.h"
#include "playlist.h"
#include "power.h"

class PlayerUI {
 public:
  void begin();
  void drawBoot(const char* msg);
  void redraw(const Playlist& pl, const AudioPlayer& ap, const BatteryStatus& bat,
              const PowerManager& power);
  void setStatus(const char* msg);
  TFT_eSPI& tft() { return tft_; }

 private:
  TFT_eSPI tft_;
  String status_;
  String lastTitle_;
  bool dirty_ = true;

  void drawBattery(const BatteryStatus& bat);
  void drawProgress(bool playing, bool paused);
};
