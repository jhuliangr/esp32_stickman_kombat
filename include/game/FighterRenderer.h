#pragma once

class Fighter;

namespace FighterRenderer {
  // Draws into Display::canvas(). Caller owns the frame lifecycle.
  void render(const Fighter& f);
}
