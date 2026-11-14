#pragma once

// ESP32-2432S028 (CYD) pin map — stock board wiring.
// Display uses HSPI; microSD uses VSPI. Do not remux those busses lightly.
//
// Audio policy: headphones only (wired I2S jack DAC + Bluetooth).
// Do NOT drive the onboard Speak (P2) speaker amp.

// --- Display (TFT / ST7789 or ILI9341) ---
static const int PIN_TFT_MISO = 12;
static const int PIN_TFT_MOSI = 13;
static const int PIN_TFT_SCLK = 14;
static const int PIN_TFT_CS   = 15;
static const int PIN_TFT_DC   = 2;
static const int PIN_TFT_BL   = 21;  // also on P3 Extended IO — shared

// --- Touch (XPT2046, separate SPI; unused — DOUT reuses touch MOSI) ---
static const int PIN_TOUCH_CLK  = 25;
static const int PIN_TOUCH_CS   = 33;
static const int PIN_TOUCH_MOSI = 32;  // reused as I2S DOUT for jack DAC
static const int PIN_TOUCH_MISO = 39;
static const int PIN_TOUCH_IRQ  = 36;

// --- microSD (VSPI) ---
static const int PIN_SD_CS   = 5;
static const int PIN_SD_MOSI = 23;
static const int PIN_SD_MISO = 19;
static const int PIN_SD_SCK  = 18;

// --- On-board peripherals ---
static const int PIN_RGB_R = 4;   // active LOW LED; also Speak-amp /SD — held inactive (amp off)
static const int PIN_RGB_G = 16;  // active LOW
static const int PIN_RGB_B = 17;  // active LOW
static const int PIN_LDR   = 34;  // ADC1_CH6 — light sensor by default
static const int PIN_SPEAK = 26;  // Speak P2 — UNUSED (leave unconnected)

// --- Extended IO P3 (labels: GND, IO35, IO22, IO21) ---
static const int PIN_BTN     = 35;  // multi-click control (input-only, needs ext pull-up)
static const int PIN_P3_IO22 = 22;  // I2S BCLK (also on CN1)
static const int PIN_P3_IO21 = 21;  // shared with backlight — avoid if BL used

// --- Wired headphone jack (external I2S DAC / headphone amp) ---
// CN1: GND, IO22, IO27, 3.3V — convenient for BCLK/LRC + power.
// DOUT uses GPIO32 (touch MOSI); touch is not used by this firmware.
static const int PIN_I2S_BCLK = 22;
static const int PIN_I2S_LRC  = 27;
static const int PIN_I2S_DOUT = 32;

// --- Optional battery sense (NOT on stock CYD) ---
// Wire a 2:1 divider from battery pack to this ADC pin after removing/isolating the LDR.
static const int PIN_BATTERY_ADC = 34;

// --- Speak amp enable (forced off) ---
// Many CYD boards gate P2 with GPIO4. We hold it inactive so Speak stays silent.
static const int PIN_SPEAK_AMP_SD = 4;
static const bool kSpeakAmpActiveHigh = true;  // HIGH = amp on for typical PAM830x
