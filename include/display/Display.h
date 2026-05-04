#pragma once

#include <Adafruit_SSD1306.h>

namespace Display {

  void init();

  void showText(const char* line1, const char* line2 = nullptr);

  // beginFrame/endFrame bracket any direct canvas() drawing so the double
  // buffer only flushes to the OLED once per frame.
  void beginFrame();
  void endFrame();
  Adafruit_SSD1306& canvas();

  // Powers down the panel (used before deep sleep).
  void sleep();

  constexpr int WIDTH  = 128;
  constexpr int HEIGHT = 64;

}
