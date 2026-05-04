#pragma once

#include <Arduino.h>

namespace PortalPages {
  // Landing page: list of available games.
  String gamesPage();
  // Player picker for Stickman Kombat (P1 / P2).
  String selectionPage();
  String controllerPage(int player);
}
