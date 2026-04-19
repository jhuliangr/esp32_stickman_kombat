#pragma once

#include "game/Fighter.h"

namespace Arena {

  void init();
  void tick(int dtMs);
  void onAction(int player, Fighter::Action a);

}
