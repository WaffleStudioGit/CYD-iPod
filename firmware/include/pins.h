#pragma once

// ESP32-2432S028 (CYD) pin map — stock board wiring.
// Display uses HSPI; microSD uses VSPI. Do not remux those busses lightly.

// --- Display (TFT / ST7789 or ILI9341) ---
static const int PIN_TFT_MISO = 12;
static const int PIN_TFT_MOSI = 13;
static const int PIN_TFT_SCLK = 14;
static const int PIN_TFT_CS   = 15;
static const int PIN_TFT_DC   = 2;
static const int PIN_TFT_BL   = 21;  // also on P3 Extended IO — shared

// --- Touch (XPT2046, separate SPI; not required for button UX) ---
static const int PIN_TOUCH_CLK  = 25;
static const int PIN_TOUCH_CS   = 33;
static const int PIN_TOUCH_MOSI = 32;
static const int PIN_TOUCH_MISO = 39;
static const int PIN_TOUCH_IRQ  = 36;

// --- microSD (VSPI) ---
static const int PIN_SD_CS   = 5;
static const int PIN_SD_MOSI = 23;
static const int PIN_SD_MISO = 19;
static const int PIN_SD_SCK  = 18;

// --- On-board peripherals ---
static const int PIN_RGB_R   = 4;   // active LOW; often also amp /SD enable
static const int PIN_RGB_G   = 16;  // active LOW
static const int PIN_RGB_B   = 17;  // active LOW
static const int PIN_LDR     = 34;  // ADC1_CH6 — light sensor by default
static const int PIN_SPEAK   = 26;  // DAC2 → Speak connector P2 (local audio)

// --- Extended IO P3 (labels: GND, IO35, IO22, IO21) ---
static const int PIN_BTN     = 35;  // multi-click control (input-only, needs ext pull-up)
static const int PIN_P3_IO22 = 22;  // free digital I/O
static const int PIN_P3_IO21 = 21;  // shared with backlight — avoid if BL used

// --- Optional battery sense (NOT on stock CYD) ---
// Wire a 2:1 divider from battery pack to this ADC pin after removing/isolating the LDR.
static const int PIN_BATTERY_ADC = 34;

// --- Amp enable (revision-dependent) ---
// Many CYD boards gate the Speak amp with GPIO4 (same net as red LED).
static const int PIN_AMP_ENABLE = 4;
