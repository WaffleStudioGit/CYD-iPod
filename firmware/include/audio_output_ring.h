#pragma once

#include "AudioOutput.h"
#include "pcm_ring.h"

// ESP8266Audio sink that pushes PCM into a ring buffer for A2DP.
class AudioOutputRing : public AudioOutput {
 public:
  explicit AudioOutputRing(PcmRing* ring) : ring_(ring) {}

  bool begin() override { return ring_ && ring_->valid(); }
  bool stop() override {
    if (ring_) ring_->clear();
    return true;
  }

  bool ConsumeSample(int16_t sample[2]) override {
    if (!ring_) return false;
    // Apply soft volume (0–100) here as well for BT path.
    int16_t s[2] = {sample[0], sample[1]};
    if (gain_ != 1.0f) {
      s[0] = (int16_t)constrain((int)(s[0] * gain_), -32768, 32767);
      s[1] = (int16_t)constrain((int)(s[1] * gain_), -32768, 32767);
    }
    // If buffer is full, drop (A2DP consumer will catch up).
    if (ring_->framesAvailable() >= 4000) {
      return true;
    }
    return ring_->writeFrames(s, 1) == 1;
  }

  void setGain(float g) { gain_ = g; }

 private:
  PcmRing* ring_ = nullptr;
  float gain_ = 1.0f;
};
