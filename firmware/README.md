# CYD-iPod firmware

Music player firmware for the **ESP32-2432S028** Cheap Yellow Display (2.8" ST7789 / ILI9341).

Gruvbox UI, microSD playback, multi-click button on **P3 IO35**, **headphones-only** audio (**wired 3.5 mm jack** via external I2S DAC/amp **or Bluetooth A2DP**), battery readout (with ADC add-on), and configurable display auto-off.

> **Audio policy:** do **not** use the onboard **Speak (P2)** speaker path. Leave P2 unconnected; the firmware keeps the Speak amp shut down.

## Quick start

```bash
cd firmware
pio run -e cyd_st7789          # build (ST7789 / CYD2USB-style panel)
pio run -e cyd_st7789 -t upload
pio device monitor -b 115200
```

If the screen is blank, garbled, or wrong colours, try the ILI9341 env:

```bash
pio run -e cyd_ili9341 -t upload
```

**Board:** PlatformIO `esp32dev` (Arduino-ESP32). Use a USB-A→C or micro-USB cable; many CYD USB-C ports lack CC resistors.

### Library / platform versions (pinned)

| Piece | Version |
| --- | --- |
| `espressif32` platform | 6.9.0 |
| TFT_eSPI | 2.5.43 |
| ESP8266Audio | 1.9.9 |
| ESP32-A2DP | v1.8.12 (git tag) |

## SD card layout

1. Format the card **FAT32** (exFAT will not mount).
2. Create a `music` folder at the root.
3. Copy `.mp3` and/or `.wav` files (folders are scanned recursively).

```
/music
  /album1
    track01.mp3
    track02.mp3
  loose-track.wav
```

If `/music` is missing, the firmware also scans the SD root for audio files.

**Tip:** Prefer 44.1 kHz stereo MP3s for Bluetooth; the A2DP path expects PCM frames at typical headset rates.

## Pin map (stock CYD)

| Function | GPIO | Notes |
| --- | --- | --- |
| TFT MOSI / MISO / SCLK | 13 / 12 / 14 | HSPI |
| TFT CS / DC / BL | 15 / 2 / 21 | Backlight also on P3 |
| SD MOSI / MISO / SCK / CS | 23 / 19 / 18 / 5 | VSPI |
| Speak (P2) | 26 | **Unused** — leave unconnected |
| Speak amp /SD (many revs) | 4 | Held **off** (also RGB red, active-LOW LED) |
| I2S BCLK (jack DAC) | **22** | CN1 / P3 |
| I2S LRC / WS (jack DAC) | **27** | CN1 |
| I2S DOUT (jack DAC) | **32** | Touch MOSI reused; touch unused |
| RGB R / G / B | 4 / 16 / 17 | Active LOW |
| LDR | 34 | ADC — reused for optional battery sense |
| Button (P3) | **35** | Input-only, **needs external pull-up** |

### Known conflicts

- Display (HSPI) and SD (VSPI) must stay on separate busses — `USE_HSPI_PORT` is set for TFT_eSPI.
- **IO21** is backlight; using it from P3 fights the display.
- **IO22** appears on both P3 and CN1 (same net).
- **IO32** is touch MOSI — this firmware does not drive the XPT2046; enabling touch later requires moving I2S DOUT.
- **IO4** is RGB red *and* Speak-amp shutdown — kept inactive so P2 stays silent.
- Classic Bluetooth (A2DP) and Wi‑Fi do not coexist well; this firmware does not enable Wi‑Fi.
- ESP32 **A2DP Source** streams *to* BT headphones. The board is **not** an A2DP sink (phone → CYD) in this build.

## Physical button (P3)

P3 labels: **GND · IO35 · IO22 · IO21**.

```
3V3 ── 10k ──┬── IO35
             │
            btn
             │
            GND  (P3 GND)
```

IO35 has **no internal pull-up**. Without the 10 kΩ resistor the pin floats and clicks will be unreliable.

| Clicks | Action |
| --- | --- |
| 1 | Play / pause |
| 2 | Next track |
| 3 | Restart current (≥3 s in) or previous track |
| 4 | Toggle output: **Jack** (wired I2S) ↔ **BT** (A2DP source) |

## Wired 3.5 mm headphones (required add-on)

**Do not** plug a speaker into **Speak P2**. That path is a small class-D speaker amp, not a headphone output, and this firmware leaves it disabled.

Jack mode streams stereo I2S from the ESP32 to an **external DAC / headphone amp**, then to a **3.5 mm TRS jack**.

### Recommended modules

| Module | Role | Notes |
| --- | --- | --- |
| **PCM5102A** (or UDA1334) I2S DAC breakout | Digital → line-level analog | Cheap, works well with ESP32 I2S |
| Small **headphone amp** (e.g. MAX97220 / “3.5 mm headphone amp” board) after the DAC | Drive 16–32 Ω phones | Prefer this over raw line-out into headphones |
| Combo “I2S DAC + 3.5 mm jack” boards | One module | Confirm it is a **headphone** or line jack, not a speaker amp (avoid MAX98357 / PAM8302 for phones) |

Avoid speaker amps (MAX98357A, PAM8301/8302, etc.) as the jack driver — they are bridged class-D outputs meant for loudspeakers and can damage headphones / sound terrible.

### Wiring (PCM5102A example)

CN1 on the CYD breaks out **GND · IO22 · IO27 · 3.3V** — use that for clocks + power. DOUT is GPIO32 (solder pad / touch MOSI net, or a jumper to the touch connector pin).

| CYD | PCM5102A (typical) |
| --- | --- |
| 3.3V | VCC |
| GND | GND |
| GPIO22 | BCK / BCLK |
| GPIO27 | LRCK / LCK / WS |
| GPIO32 | DIN / DATA |
| — | SCK / FLT / XMT per module docs (often tie XMT high to unmute; SCK to GND for internal PLL) |

Then:

```
PCM5102 LOUT/ROUT ──► headphone amp IN ──► 3.5 mm tip/ring
GND shared throughout                 sleeve ── GND
```

If your DAC board already has a safe headphone jack and series protection, you may omit a separate amp — keep volume moderate on first listen.

Firmware path: `AudioOutMode::WiredJack` → ESP8266Audio → **external I2S** on pins 22 / 27 / 32.

## Bluetooth headphones

- Mode: **A2DP Source** (CYD plays SD music → your **BT headphones**).
- Put the headset in pairing mode, then **4-click** to switch to **BT** (or set `DEFAULT_AUDIO_OUT_MODE` to `Bluetooth` in `include/config.h`).
- Optional name filter: set `BT_SINK_NAME_FILTER` to a substring of the headset name; empty accepts the first discovered sink.
- Pairing notes: classic BT only (ESP32-WROOM). SBC codec. Re-enter BT mode or reboot if the sink was offline at discovery.

## Battery status

| Situation | UI |
| --- | --- |
| Stock board (default) | Shows **USB** — no pack ADC on the PCB |
| With divider add-on | Shows **percent** |

To enable sensing:

1. Build a resistive divider from the pack (e.g. 100k + 100k → ratio 2.0) into **GPIO34**.
2. Isolate or remove the LDR so it does not load the ADC node.
3. Keep ADC voltage ≤ 3.3 V.
4. In `include/config.h` set:

```cpp
#define BATTERY_SENSE_ENABLED 1
#define BATTERY_DIVIDER_RATIO 2.0f
#define BATTERY_ADC_EMPTY_MV  3300
#define BATTERY_ADC_FULL_MV   4200
```

## Auto-off

Configured in `include/config.h`:

| Macro | Default | Meaning |
| --- | --- | --- |
| `IDLE_DISPLAY_OFF_MS` | 60000 | Backlight off after idle (0 = never) |
| `IDLE_DEEP_SLEEP_MS` | 300000 | Deep sleep after backlight has been off this long (0 = never) |

Any button click resets the idle timer and wakes the backlight. Deep sleep wakes on IO35 going LOW (button press).

## Stock board vs extras

| Feature | Stock CYD | Needs extra hardware |
| --- | --- | --- |
| Gruvbox player UI | Yes | — |
| microSD MP3/WAV | Yes (FAT32 card) | — |
| Multi-click button | Code ready on IO35 | Switch + **10k pull-up** on P3 |
| Onboard Speak (P2) | **Not used** | Leave unconnected |
| Wired 3.5 mm headphones | I2S software path | **I2S DAC + headphone amp + jack** |
| Bluetooth headphones | A2DP **source** software | BT headphones (sink) |
| Phone → CYD BT sink | Not implemented | Different firmware |
| Battery % | Shows USB placeholder | Voltage divider → GPIO34 + `BATTERY_SENSE_ENABLED` |
| Auto-off / deep sleep | Yes | — |

## Project layout

```
firmware/
  platformio.ini
  include/     pins, config, theme, module headers
  src/         main, ui, audio, playlist, button, battery, power
```

## License

Same as the repository unless noted otherwise.
