#include <Arduino.h>

#include "display/Display.h"
#include "net/Portal.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n[main] Booting...");

  Display::init();
  Portal::init();
  Display::showText("Connect to:", Portal::SSID);
}

void loop() {
  Portal::loop();
}
