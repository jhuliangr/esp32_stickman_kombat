#pragma once

#include "game/Fighter.h"

namespace Game {

  void init(const char* visibleSsid);
  void tick(unsigned long now);

  // Called by Portal when a player opens a pad page; flips the splash off
  // and starts the combat loop.
  void notifyControllerOpened();

  void onPlayerAction(int player, Fighter::Action a);

  bool isInCombat();

  // Closes any active match and returns to the splash screen.
  void reset();

}
