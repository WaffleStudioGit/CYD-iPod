#pragma once

#include <Arduino.h>
#include "config.h"
#include "playlist.h"

class AudioPlayer {
 public:
  void begin(Playlist* playlist);
  void update();  // pump decoder

  void playPause();
  void next();
  void prevOrRestart();
  void stop();

  bool isPlaying() const { return playing_; }
  bool isPaused() const { return paused_; }
  AudioOutMode mode() const { return mode_; }
  void setMode(AudioOutMode mode);
  void cycleMode();

  uint8_t volume() const { return volume_; }
  void setVolume(uint8_t v);
  void volumeUp();
  void volumeDown();

  bool btConnected() const { return btConnected_; }
  const char* modeLabel() const;
  const char* statusLabel() const;

 private:
  Playlist* playlist_ = nullptr;
  AudioOutMode mode_ = DEFAULT_AUDIO_OUT_MODE;
  bool playing_ = false;
  bool paused_ = false;
  bool btConnected_ = false;
  bool btStarted_ = false;
  uint8_t volume_ = kDefaultVolume;
  uint32_t trackStartMs_ = 0;
  bool restartRequested_ = false;

  bool openCurrent();
  void closeDecoder();
  void applyVolume();
  void forceSpeakAmpOff();
  void ensureBt();
  void teardownBt();
};
