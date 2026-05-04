#include <Arduino.h>

#include "display/Display.h"
#include "net/Portal.h"
#include "net/Sockets.h"
#include "game/Game.h"
#include "audio/Audio.h"
#include "power/Power.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n[main] Booting...");

  Display::init();
  Audio::init();
  Power::init();
  Portal::init();
  Sockets::init();
  Game::init(Portal::SSID);
}

void loop() {
  unsigned long now = millis();
  Portal::loop();
  Sockets::loop();
  Game::tick(now);
  Audio::tick(now);
  Power::tick(now);
}
