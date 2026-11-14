#pragma once

#include <Arduino.h>

// ----- Product identity -----
static const char* const kDeviceName = "CYD-iPod";
static const char* const kBtSourceName = "CYD-iPod";

// ----- SD layout -----
// Put tracks under /music (recursive). Supported: .mp3, .wav
static const char* const kMusicRoot = "/music";

// ----- Button (P3 IO35) -----
static const uint16_t kBtnDebounceMs = 40;
static const uint16_t kBtnMultiClickWindowMs = 450;
static const bool kBtnActiveLow = true;  // switch to GND, external pull-up to 3V3

// ----- Auto-off (idle) -----
// Display backlight dims/off after this idle period. 0 = never.
#ifndef IDLE_DISPLAY_OFF_MS
#define IDLE_DISPLAY_OFF_MS 60000UL
#endif
// Deep sleep after longer idle (backlight already off). 0 = never deep-sleep.
#ifndef IDLE_DEEP_SLEEP_MS
#define IDLE_DEEP_SLEEP_MS 300000UL
#endif

// ----- Audio output mode -----
enum class AudioOutMode : uint8_t {
  LocalDac = 0,  // Speak P2 / 3.5 mm jack amp path on GPIO26
  Bluetooth = 1  // A2DP Source → paired BT headphones/speaker
};

// Default start mode. Change at runtime via 4-click on the button.
#ifndef DEFAULT_AUDIO_OUT_MODE
#define DEFAULT_AUDIO_OUT_MODE AudioOutMode::LocalDac
#endif

// Preferred A2DP sink name (partial match). Empty = connect to first found sink.
#ifndef BT_SINK_NAME_FILTER
#define BT_SINK_NAME_FILTER ""
#endif

// ----- Battery sense (hardware add-on) -----
// Stock CYD has no pack ADC. Enable only after wiring a divider to PIN_BATTERY_ADC.
#ifndef BATTERY_SENSE_ENABLED
#define BATTERY_SENSE_ENABLED 0
#endif
// Divider: Vbat -- R1 -- ADC -- R2 -- GND.  Scale = (R1+R2)/R2.
#ifndef BATTERY_DIVIDER_RATIO
#define BATTERY_DIVIDER_RATIO 2.0f
#endif
#ifndef BATTERY_ADC_FULL_MV
#define BATTERY_ADC_FULL_MV 4200
#endif
#ifndef BATTERY_ADC_EMPTY_MV
#define BATTERY_ADC_EMPTY_MV 3300
#endif

// ----- Volume -----
static const uint8_t kDefaultVolume = 60;  // 0–100
static const uint8_t kVolumeStep = 5;

// ----- UI refresh -----
static const uint16_t kUiTickMs = 200;
