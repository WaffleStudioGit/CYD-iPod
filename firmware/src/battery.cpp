#include "battery.h"
#include "config.h"
#include "pins.h"

void BatteryMonitor::begin() {
  last_.senseEnabled = (BATTERY_SENSE_ENABLED != 0);
  last_.valid = false;
  last_.millivolts = 0;
  last_.percent = 0;
  last_.label = BATTERY_SENSE_ENABLED ? "--%" : "USB";
#if BATTERY_SENSE_ENABLED
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(PIN_BATTERY_ADC, INPUT);
#endif
}

BatteryStatus BatteryMonitor::read() {
#if !BATTERY_SENSE_ENABLED
  last_.senseEnabled = false;
  last_.valid = false;
  last_.label = "USB";
  return last_;
#else
  // Average a few samples; convert to pack mV via divider ratio.
  uint32_t sum = 0;
  for (int i = 0; i < 8; ++i) {
    sum += analogRead(PIN_BATTERY_ADC);
  }
  const float adc = sum / 8.0f;
  // ESP32 ADC ~0–3.3 V at 11 dB (rough; calibrate for your divider).
  const float pinMv = (adc / 4095.0f) * 3300.0f;
  const uint16_t packMv = (uint16_t)(pinMv * BATTERY_DIVIDER_RATIO + 0.5f);

  int pct = (int)((packMv - BATTERY_ADC_EMPTY_MV) * 100.0f /
                  (float)(BATTERY_ADC_FULL_MV - BATTERY_ADC_EMPTY_MV));
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;

  last_.senseEnabled = true;
  last_.valid = true;
  last_.millivolts = packMv;
  last_.percent = (uint8_t)pct;
  static char buf[8];
  snprintf(buf, sizeof(buf), "%u%%", last_.percent);
  last_.label = buf;
  return last_;
#endif
}
