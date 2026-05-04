#include "game/Game.h"
#include "game/Arena.h"
#include "display/Display.h"
#include "audio/Audio.h"
#include "net/Sockets.h"

#include <Arduino.h>

namespace {
  bool          combatStarted = false;
  unsigned long lastTick      = 0;
  const char*   savedSsid     = "";

  // ~30 FPS cap. Keeps the I2C flush from saturating the link on fast boards.
  constexpr int MIN_FRAME_MS = 33;

  void startIfNeeded(const char* reason) {
    if (combatStarted) return;
    combatStarted = true;
    lastTick      = millis();
    Serial.printf("[Game] Combat started (%s)\n", reason);
    Audio::playStart();
  }
}

namespace Game {

  void init(const char* visibleSsid) {
    savedSsid     = visibleSsid;
    combatStarted = false;
    lastTick      = millis();
    Arena::init();
    Display::showText("Connect to:", savedSsid);
  }

  void notifyControllerOpened() {
    startIfNeeded("controller opened");
  }

  void onPlayerAction(int player, Fighter::Action a) {
    startIfNeeded("action received");
    Arena::onAction(player, a);
  }

  void tick(unsigned long now) {
    if (!combatStarted) return;

    int dt = (int)(now - lastTick);
    if (dt < MIN_FRAME_MS) return;
    lastTick = now;

    Arena::tick(dt);
  }

  bool isInCombat() { return combatStarted; }

  void reset() {
    Sockets::closeAll();
    init(savedSsid);
  }

}
