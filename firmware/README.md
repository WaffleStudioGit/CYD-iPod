# CYD-iPod firmware

Music player firmware for the **ESP32-2432S028** Cheap Yellow Display (2.8" ST7789 / ILI9341).

Gruvbox UI, microSD playback, multi-click button on **P3 IO35**, local audio on **Speak P2** (3.5 mm jack with external amp), optional **Bluetooth A2DP source**, battery readout (with ADC add-on), and configurable display auto-off.

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
| Speak (DAC → P2) | **26** | Local audio |
| Amp enable (many revs) | 4 | Shared with RGB red (active-LOW LED) |
| RGB R / G / B | 4 / 16 / 17 | Active LOW |
| LDR | 34 | ADC — reused for optional battery sense |
| Button (P3) | **35** | Input-only, **needs external pull-up** |
| Free on P3 / CN1 | 22, 27 | Digital I/O |
| Touch SPI | 25/32/39/33/36 | Unused by this firmware |

### Known conflicts

- Display (HSPI) and SD (VSPI) must stay on separate busses — `USE_HSPI_PORT` is set for TFT_eSPI.
- **IO21** is backlight; using it from P3 fights the display.
- **IO22** appears on both P3 and CN1 (same net).
- **IO4** is RGB red *and* often the speaker-amp shutdown — local playback drives it HIGH.
- Classic Bluetooth (A2DP) and Wi‑Fi do not coexist well; this firmware does not enable Wi‑Fi.
- ESP32 **A2DP Source** streams *to* headphones/speakers. The board is **not** an A2DP sink (phone → CYD) in this build.

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
| 4 | Toggle output: **Jack** (local DAC) ↔ **BT** (A2DP source) |

## 3.5 mm jack / Speak P2

Stock CYD brings **GPIO26 DAC** to the 2-pin **Speak (P2)** connector through a small amp (PAM8301/8302-class on many boards).

### Minimal speaker

Wire a 4–8 Ω speaker (or the included buzz/speaker) across P2. Observe polarity if marked.

### Headphone jack (recommended add-on)

Do **not** hang low-Z headphones straight on the DAC pin without a proper amp / series protection — you can overload the ESP32 DAC and get tiny volume.

Recommended:

1. Take **audio** from Speak P2 (post-amp) *or* feed GPIO26 into an external headphone amp module (e.g. MAX98357A is I2S — not drop-in; prefer a **3.5 mm breakout with its own amp** fed from P2, or a PAM8302 breakout → jack).
2. Simple wired path many people use: **P2 → 100–220 µF series cap → tip of 3.5 mm jack**, sleeve to GND, with a **~1 kΩ** series resistor for safety. Volume will be low; an external amp is strongly preferred.
3. Share **GND** with the CYD. Power external amps from **5 V (VIN)** or **3V3** per module ratings.

Firmware path: `AudioOutMode::LocalDac` → ESP8266Audio → internal DAC → GPIO26. Amp enable on GPIO4 is asserted while playing.

## Bluetooth audio

- Mode: **A2DP Source** (CYD plays SD music → your BT headphones/speaker).
- Put the sink in pairing mode, then **4-click** to switch to **BT** (or set `DEFAULT_AUDIO_OUT_MODE` to `Bluetooth` in `include/config.h`).
- Optional name filter: set `BT_SINK_NAME_FILTER` to a substring of your speaker’s name; empty accepts the first discovered sink.
- Pairing notes: classic BT only (ESP32-WROOM). Range and codec quality match the ESP-IDF A2DP source stack (SBC). Re-enter BT mode or reboot if the sink was offline at discovery.

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
| Local audio (Speak) | Yes on P2 / GPIO26 | Speaker or jack amp |
| 3.5 mm headphones | Software path = local DAC | Jack + **amp** (recommended) |
| Bluetooth to headset | A2DP **source** software | BT headphones/speaker (sink) |
| Phone → CYD BT sink | Not implemented | Would need different A2DP sink firmware + usually no local decode path |
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
