#include "audio_player.h"
#include "pins.h"
#include "pcm_ring.h"
#include "audio_output_ring.h"

#include <SD.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "BluetoothA2DPSource.h"

namespace {

AudioFileSourceSD* fileSrc = nullptr;
AudioGenerator* generator = nullptr;
AudioOutputI2S* i2sOut = nullptr;
AudioOutputRing* ringOut = nullptr;
PcmRing* pcmRing = nullptr;
BluetoothA2DPSource* a2dp = nullptr;
AudioPlayer* gPlayer = nullptr;

bool isMp3(const String& path) {
  String l = path;
  l.toLowerCase();
  return l.endsWith(".mp3");
}

bool isWav(const String& path) {
  String l = path;
  l.toLowerCase();
  return l.endsWith(".wav");
}

bool sinkFilter(const char* ssid, esp_bd_addr_t /*addr*/, int /*rssi*/) {
  const char* filter = BT_SINK_NAME_FILTER;
  if (filter == nullptr || filter[0] == '\0') {
    return true;  // accept first A2DP sink discovered
  }
  return strstr(ssid, filter) != nullptr;
}

int32_t a2dpDataCb(Frame* frame, int32_t frameCount) {
  if (!pcmRing) {
    for (int32_t i = 0; i < frameCount; ++i) {
      frame[i].channel1 = 0;
      frame[i].channel2 = 0;
    }
    return frameCount;
  }
  return pcmRing->readToFrames(frame, frameCount);
}

}  // namespace

void AudioPlayer::begin(Playlist* playlist) {
  playlist_ = playlist;
  gPlayer = this;
  mode_ = DEFAULT_AUDIO_OUT_MODE;
  volume_ = kDefaultVolume;

  pinMode(PIN_AMP_ENABLE, OUTPUT);
  setAmpEnabled(false);

  pcmRing = new PcmRing(4096);
  ringOut = new AudioOutputRing(pcmRing);

  // Local path: internal DAC (GPIO25 L / GPIO26 R). Speak P2 is GPIO26.
  i2sOut = new AudioOutputI2S(0, 1 /* INTERNAL_DAC */);
  i2sOut->SetOutputModeMono(true);
  applyVolume();
}

void AudioPlayer::setAmpEnabled(bool on) {
  // Many CYD revisions use GPIO4 as amp _SD (active HIGH enable).
  // RGB red shares this pin (LED active LOW) — red LED off while amp enabled.
  digitalWrite(PIN_AMP_ENABLE, on ? HIGH : LOW);
}

void AudioPlayer::applyVolume() {
  const float gain = volume_ / 100.0f;
  if (i2sOut) i2sOut->SetGain(gain);
  if (ringOut) ringOut->setGain(gain);
}

void AudioPlayer::setVolume(uint8_t v) {
  if (v > 100) v = 100;
  volume_ = v;
  applyVolume();
}

void AudioPlayer::volumeUp() { setVolume((uint8_t)min(100, (int)volume_ + kVolumeStep)); }
void AudioPlayer::volumeDown() { setVolume((uint8_t)max(0, (int)volume_ - kVolumeStep)); }

const char* AudioPlayer::modeLabel() const {
  return mode_ == AudioOutMode::Bluetooth ? "BT" : "Jack";
}

const char* AudioPlayer::statusLabel() const {
  if (!playlist_ || playlist_->empty()) return "No music";
  if (mode_ == AudioOutMode::Bluetooth && !btConnected_) return "BT pairing…";
  if (paused_) return "Paused";
  if (playing_) return "Playing";
  return "Stopped";
}

void AudioPlayer::ensureBt() {
  if (btStarted_) return;
  if (!a2dp) a2dp = new BluetoothA2DPSource();
  a2dp->set_ssid_callback(sinkFilter);
  a2dp->set_auto_reconnect(true);
  a2dp->start(a2dpDataCb);
  // Advertise-ish local name used in logs; sink discovery uses ssid callback.
  btStarted_ = true;
}

void AudioPlayer::teardownBt() {
  if (a2dp && btStarted_) {
    a2dp->end();
    btStarted_ = false;
    btConnected_ = false;
  }
}

void AudioPlayer::setMode(AudioOutMode mode) {
  if (mode_ == mode) return;
  const bool wasPlaying = playing_ && !paused_;
  stop();
  mode_ = mode;
  if (mode_ == AudioOutMode::Bluetooth) {
    setAmpEnabled(false);
    ensureBt();
  } else {
    teardownBt();
  }
  if (wasPlaying) {
    openCurrent();
  }
}

void AudioPlayer::cycleMode() {
  setMode(mode_ == AudioOutMode::LocalDac ? AudioOutMode::Bluetooth
                                          : AudioOutMode::LocalDac);
}

void AudioPlayer::closeDecoder() {
  if (generator) {
    if (generator->isRunning()) generator->stop();
    delete generator;
    generator = nullptr;
  }
  if (fileSrc) {
    delete fileSrc;
    fileSrc = nullptr;
  }
  if (pcmRing) pcmRing->clear();
}

bool AudioPlayer::openCurrent() {
  closeDecoder();
  if (!playlist_ || playlist_->empty()) {
    playing_ = false;
    paused_ = false;
    setAmpEnabled(false);
    return false;
  }

  const String& path = playlist_->current();
  fileSrc = new AudioFileSourceSD(path.c_str());

  AudioOutput* out = nullptr;
  if (mode_ == AudioOutMode::Bluetooth) {
    ensureBt();
    if (pcmRing) pcmRing->clear();
    out = ringOut;
    setAmpEnabled(false);
  } else {
    out = i2sOut;
    setAmpEnabled(true);
  }

  if (isMp3(path)) {
    generator = new AudioGeneratorMP3();
  } else if (isWav(path)) {
    generator = new AudioGeneratorWAV();
  } else {
    closeDecoder();
    return false;
  }

  if (!generator->begin(fileSrc, out)) {
    Serial.printf("Audio: failed to start %s\n", path.c_str());
    closeDecoder();
    setAmpEnabled(false);
    playing_ = false;
    return false;
  }

  playing_ = true;
  paused_ = false;
  trackStartMs_ = millis();
  restartRequested_ = false;
  applyVolume();
  Serial.printf("Audio: playing %s via %s\n", path.c_str(), modeLabel());
  return true;
}

void AudioPlayer::playPause() {
  if (!playlist_ || playlist_->empty()) return;

  if (!playing_) {
    openCurrent();
    return;
  }
  if (paused_) {
    paused_ = false;
    if (mode_ == AudioOutMode::LocalDac) setAmpEnabled(true);
    return;
  }
  paused_ = true;
  setAmpEnabled(false);
}

void AudioPlayer::next() {
  if (!playlist_ || playlist_->empty()) return;
  playlist_->next();
  openCurrent();
}

void AudioPlayer::prevOrRestart() {
  if (!playlist_ || playlist_->empty()) return;
  // Triple-click: if >3s into track, restart; else previous track.
  const uint32_t elapsed = millis() - trackStartMs_;
  if (playing_ && elapsed > 3000) {
    openCurrent();
    return;
  }
  playlist_->prev();
  openCurrent();
}

void AudioPlayer::stop() {
  closeDecoder();
  playing_ = false;
  paused_ = false;
  setAmpEnabled(false);
}

void AudioPlayer::update() {
  if (a2dp && btStarted_) {
    btConnected_ = a2dp->is_connected();
  }

  if (!playing_ || paused_ || !generator) return;

  if (!generator->isRunning()) {
    // Track finished → advance
    playlist_->next();
    openCurrent();
    return;
  }

  if (!generator->loop()) {
    generator->stop();
    playlist_->next();
    openCurrent();
  }
}
