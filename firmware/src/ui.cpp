#include "ui.h"
#include "theme.h"
#include "config.h"

void PlayerUI::begin() {
  tft_.init();
  tft_.setRotation(1);  // landscape 320×240
  tft_.fillScreen(Gruvbox::bg0);
  tft_.setTextColor(Gruvbox::fg1, Gruvbox::bg0);
  status_ = "Boot";
  dirty_ = true;
}

void PlayerUI::drawBoot(const char* msg) {
  tft_.fillScreen(Gruvbox::bg0);
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(Gruvbox::orange, Gruvbox::bg0);
  tft_.drawString(kDeviceName, 160, 90, 4);
  tft_.setTextColor(Gruvbox::fg2, Gruvbox::bg0);
  tft_.drawString(msg, 160, 130, 2);
  tft_.setTextDatum(TL_DATUM);
}

void PlayerUI::setStatus(const char* msg) {
  status_ = msg;
  dirty_ = true;
}

void PlayerUI::drawBattery(const BatteryStatus& bat) {
  const int x = 250;
  const int y = 8;
  const int w = 60;
  const int h = 16;
  tft_.drawRoundRect(x, y, w - 4, h, 2, Gruvbox::fg2);
  tft_.fillRect(x + w - 4, y + 4, 3, h - 8, Gruvbox::fg2);

  uint16_t fillColor = Gruvbox::green;
  int fillW = 0;
  if (bat.senseEnabled && bat.valid) {
    fillW = (int)((w - 8) * bat.percent / 100);
    if (bat.percent <= 20) fillColor = Gruvbox::red;
    else if (bat.percent <= 40) fillColor = Gruvbox::yellow;
  } else {
    // USB / unknown — show full aqua bar
    fillW = w - 8;
    fillColor = Gruvbox::aqua;
  }
  if (fillW > 0) {
    tft_.fillRoundRect(x + 2, y + 2, fillW, h - 4, 1, fillColor);
  }
  tft_.setTextDatum(MR_DATUM);
  tft_.setTextColor(Gruvbox::fg0, Gruvbox::bg0);
  tft_.drawString(bat.label, x - 4, y + h / 2, 2);
  tft_.setTextDatum(TL_DATUM);
}

void PlayerUI::drawProgress(bool playing, bool paused) {
  const int y = 200;
  tft_.fillRect(20, y, 280, 6, Gruvbox::bg2);
  if (playing) {
    // Indeterminate-ish pulse based on millis when we lack duration metadata
    const int pulse = (int)((millis() / 30) % 280);
    const int bar = paused ? 40 : 80;
    tft_.fillRect(20 + pulse, y, bar, 6, paused ? Gruvbox::yellow : Gruvbox::aqua);
  }
}

void PlayerUI::redraw(const Playlist& pl, const AudioPlayer& ap, const BatteryStatus& bat,
                      const PowerManager& power) {
  if (!power.displayOn()) return;

  tft_.fillScreen(Gruvbox::bg0);

  // Top bar
  tft_.fillRect(0, 0, 320, 28, Gruvbox::bg1);
  tft_.setTextColor(Gruvbox::orange, Gruvbox::bg1);
  tft_.setTextDatum(ML_DATUM);
  tft_.drawString(kDeviceName, 8, 14, 2);

  tft_.setTextColor(Gruvbox::blue, Gruvbox::bg1);
  tft_.drawString(ap.modeLabel(), 120, 14, 2);

  drawBattery(bat);

  // Now playing
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(Gruvbox::fg2, Gruvbox::bg0);
  tft_.drawString("NOW PLAYING", 160, 50, 2);

  tft_.setTextColor(Gruvbox::fg0, Gruvbox::bg0);
  String title = pl.empty() ? String("Insert SD: /music/*.mp3") : String(pl.currentName());
  if (title.length() > 28) {
    title = title.substring(0, 25) + "...";
  }
  tft_.drawString(title, 160, 88, 4);

  // Track index
  if (!pl.empty()) {
    char idx[32];
    snprintf(idx, sizeof(idx), "%d / %u", pl.index() + 1, (unsigned)pl.size());
    tft_.setTextColor(Gruvbox::gray, Gruvbox::bg0);
    tft_.drawString(idx, 160, 120, 2);
  }

  // Status + volume
  tft_.setTextColor(Gruvbox::green, Gruvbox::bg0);
  tft_.drawString(ap.statusLabel(), 160, 150, 2);

  char vol[24];
  snprintf(vol, sizeof(vol), "Vol %u%%", ap.volume());
  tft_.setTextColor(Gruvbox::fg1, Gruvbox::bg0);
  tft_.drawString(vol, 160, 172, 2);

  drawProgress(ap.isPlaying(), ap.isPaused());

  // Footer help
  tft_.fillRect(0, 220, 320, 20, Gruvbox::bg1);
  tft_.setTextColor(Gruvbox::fg2, Gruvbox::bg1);
  tft_.setTextDatum(MC_DATUM);
  tft_.drawString("1:play  2:next  3:prev  4:BT/Jack", 160, 230, 1);
  tft_.setTextDatum(TL_DATUM);

  dirty_ = false;
  lastTitle_ = title;
}
