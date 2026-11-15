#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Simple stereo int16 ring buffer shared by decoder → A2DP callback.
class PcmRing {
 public:
  explicit PcmRing(size_t frames = 4096)
      : capacity_(frames),
        buf_((int16_t*)heap_caps_malloc(frames * 2 * sizeof(int16_t), MALLOC_CAP_8BIT)) {
    mutex_ = xSemaphoreCreateMutex();
  }

  ~PcmRing() {
    if (buf_) free(buf_);
    if (mutex_) vSemaphoreDelete(mutex_);
  }

  bool valid() const { return buf_ != nullptr && mutex_ != nullptr; }

  void clear() {
    if (!mutex_) return;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    head_ = tail_ = count_ = 0;
    xSemaphoreGive(mutex_);
  }

  // Write interleaved L,R frames. Returns frames written.
  size_t writeFrames(const int16_t* interleaved, size_t frames) {
    if (!buf_ || !mutex_) return 0;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    size_t written = 0;
    while (written < frames && count_ < capacity_) {
      buf_[head_ * 2] = interleaved[written * 2];
      buf_[head_ * 2 + 1] = interleaved[written * 2 + 1];
      head_ = (head_ + 1) % capacity_;
      count_++;
      written++;
    }
    xSemaphoreGive(mutex_);
    return written;
  }

  // Read into A2DP Frame-like channel1/channel2 arrays via callback style.
  template <typename FrameT>
  int32_t readToFrames(FrameT* frame, int32_t frameCount) {
    if (!buf_ || !mutex_) return 0;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    int32_t got = 0;
    while (got < frameCount && count_ > 0) {
      frame[got].channel1 = buf_[tail_ * 2];
      frame[got].channel2 = buf_[tail_ * 2 + 1];
      tail_ = (tail_ + 1) % capacity_;
      count_--;
      got++;
    }
    xSemaphoreGive(mutex_);
    // Underrun: pad silence
    for (int32_t i = got; i < frameCount; ++i) {
      frame[i].channel1 = 0;
      frame[i].channel2 = 0;
    }
    return frameCount;
  }

  size_t framesAvailable() {
    if (!mutex_) return 0;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    size_t c = count_;
    xSemaphoreGive(mutex_);
    return c;
  }

 private:
  size_t capacity_ = 0;
  int16_t* buf_ = nullptr;
  size_t head_ = 0;
  size_t tail_ = 0;
  size_t count_ = 0;
  SemaphoreHandle_t mutex_ = nullptr;
};
