#include <Arduino.h>

#include "display/Display.h"
#include "net/Portal.h"
#include "game/Game.h"
#include "audio/Audio.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n[main] Booting...");

  Display::init();
  Audio::init();
  Portal::init();
  Game::init(Portal::SSID);
}

void loop() {
  unsigned long now = millis();
  Portal::loop();
  Game::tick(now);
  Audio::tick(now);
}
