#include <Arduino.h>

#include "display/Display.h"
#include "net/Portal.h"
#include "game/Game.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n[main] Booting...");

  Display::init();
  Portal::init();
  Game::init(Portal::SSID);
}

void loop() {
  Portal::loop();
  Game::tick(millis());
}
