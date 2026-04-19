#include <Arduino.h>

#include "display/Display.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n[main] Booting...");

  Display::init();
  Display::showText("ESP32", "ready");
}

void loop() {
}
