#pragma once

#include <Arduino.h>

// Gruvbox Dark hard — 16-bit RGB565 for TFT_eSPI
namespace Gruvbox {
  static const uint16_t bg0    = 0x2145;  // #1d2021
  static const uint16_t bg1    = 0x2926;  // #282828
  static const uint16_t bg2    = 0x31A6;  // #3c3836
  static const uint16_t fg0    = 0xEF5A;  // #fbf1c7
  static const uint16_t fg1    = 0xD6B9;  // #ebdbb2
  static const uint16_t fg2    = 0xA514;  // #a89984
  static const uint16_t gray   = 0x9492;  // #928374
  static const uint16_t red    = 0xD9E7;  // #fb4934
  static const uint16_t green  = 0xB723;  // #b8bb26
  static const uint16_t yellow = 0xE70A;  // #fabd2f
  static const uint16_t blue   = 0x74DF;  // #83a598
  static const uint16_t purple = 0xD3B6;  // #d3869b
  static const uint16_t aqua   = 0x8E8C;  // #8ec07c
  static const uint16_t orange = 0xE408;  // #fe8019
}
