#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "pins.h"
#include "config.h"
#include "button.h"
#include "battery.h"
#include "power.h"
#include "playlist.h"
#include "audio_player.h"
#include "ui.h"

namespace {
PlayerUI ui;
MultiClickButton button;
BatteryMonitor battery;
PowerManager power;
Playlist playlist;
AudioPlayer player;
SPIClass sdSPI(VSPI);

uint32_t lastUiMs = 0;
bool sdOk = false;
}  // namespace

static bool initSd() {
  sdSPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (!SD.begin(PIN_SD_CS, sdSPI)) {
    Serial.println("SD: mount failed");
    return false;
  }
  Serial.println("SD: mounted");
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("\n=== %s ===\n", kDeviceName);

  // RGB LEDs off (active LOW)
  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);
  digitalWrite(PIN_RGB_R, HIGH);
  digitalWrite(PIN_RGB_G, HIGH);
  digitalWrite(PIN_RGB_B, HIGH);

  ui.begin();
  ui.drawBoot("Starting…");
  power.begin(PIN_TFT_BL);
  battery.begin();
  button.begin(PIN_BTN, kBtnActiveLow);

  ui.drawBoot("Mounting SD…");
  sdOk = initSd();
  if (sdOk) {
    if (!playlist.begin(kMusicRoot)) {
      Serial.println("SD: no tracks under /music (or root)");
      ui.setStatus("No tracks in /music");
    } else {
      Serial.printf("SD: %u tracks\n", (unsigned)playlist.size());
    }
  } else {
    ui.setStatus("SD mount failed");
  }

  player.begin(&playlist);
  if (sdOk && !playlist.empty()) {
    // Auto-start first track on local DAC
    player.playPause();
  }

  ui.redraw(playlist, player, battery.read(), power);
  lastUiMs = millis();
}

void loop() {
  button.update();
  const ButtonAction action = button.consume();
  if (action != ButtonAction::None) {
    power.poke();
    switch (action) {
      case ButtonAction::Single:
        player.playPause();
        break;
      case ButtonAction::Double:
        player.next();
        break;
      case ButtonAction::Triple:
        player.prevOrRestart();
        break;
      case ButtonAction::Quad:
        player.cycleMode();
        break;
      default:
        break;
    }
    ui.redraw(playlist, player, battery.read(), power);
  }

  player.update();
  power.update();

  const uint32_t now = millis();
  if (now - lastUiMs >= kUiTickMs) {
    lastUiMs = now;
    // Refresh battery + progress + BT status
    ui.redraw(playlist, player, battery.read(), power);
  }

  // Yield a little so BT / idle tasks run
  delay(1);
}
