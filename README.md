# CYD-iPod

Turns a 2.8" Cheap Yellow Display (**ESP32-2432S028**, ST7789) into a portable music player.

Firmware lives in [`firmware/`](firmware/) — PlatformIO project with gruvbox UI, microSD playback, P3 multi-click button, Speak/jack audio path, Bluetooth A2DP source, battery readout hook, and idle auto-off.

## Flash

```bash
cd firmware
pio run -e cyd_st7789 -t upload
```

See **[firmware/README.md](firmware/README.md)** for SD layout (`/music/*.mp3`), pin map, button/jack wiring, BT pairing, and stock vs add-on hardware notes.
