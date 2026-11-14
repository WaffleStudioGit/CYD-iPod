# CYD-iPod

Turns a 2.8" Cheap Yellow Display (**ESP32-2432S028**, ST7789) into a portable music player.

Firmware lives in [`firmware/`](firmware/) — PlatformIO project with gruvbox UI, microSD playback, P3 multi-click button, **headphones-only** audio (wired 3.5 mm I2S jack **or** Bluetooth A2DP), battery readout hook, and idle auto-off.

**Not used:** onboard Speak (P2) speaker path — leave it unconnected.

## Flash

```bash
cd firmware
pio run -e cyd_st7789 -t upload
```

See **[firmware/README.md](firmware/README.md)** for SD layout (`/music/*.mp3`), pin map, button wiring, **headphone jack DAC/amp** wiring, BT pairing, and stock vs add-on hardware notes.
