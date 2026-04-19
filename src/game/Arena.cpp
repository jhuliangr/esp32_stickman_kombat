#include "game/Arena.h"
#include "display/Display.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

namespace {
  constexpr int START_X_P1 = 30;
  constexpr int START_X_P2 = Display::WIDTH - 30;

  constexpr int FLOOR_Y    = 60;
  constexpr int HUD_HEIGHT = 6;

  constexpr int RESTART_MS = 2500;

  Fighter* p1 = nullptr;
  Fighter* p2 = nullptr;

  bool koActive = false;
  int  koTimer  = 0;
  int  winner   = 0;

  int  winsP1 = 0;
  int  winsP2 = 0;

  void drawHpBar(int xLeft, int hp) {
    Adafruit_SSD1306& g = Display::canvas();
    constexpr int BAR_W = 56;
    g.drawRect(xLeft, 0, BAR_W, HUD_HEIGHT, SSD1306_WHITE);
    int filled = (hp * (BAR_W - 2)) / 100;
    if (filled > 0) g.fillRect(xLeft + 1, 1, filled, HUD_HEIGHT - 2, SSD1306_WHITE);
  }

  void drawHUD() {
    drawHpBar(2, p1->getHP());
    drawHpBar(Display::WIDTH - 58, p2->getHP());
  }

  void drawFloor() {
    Adafruit_SSD1306& g = Display::canvas();
    g.drawFastHLine(0, FLOOR_Y, Display::WIDTH, SSD1306_WHITE);
  }

  int centerX(int textPxWidth) {
    int x = (Display::WIDTH - textPxWidth) / 2;
    return x < 0 ? 0 : x;
  }

  void drawKoBanner() {
    Adafruit_SSD1306& g = Display::canvas();

    constexpr int CHAR_W_SIZE2 = 12;
    constexpr int CHAR_W_SIZE3 = 18;

    g.setTextSize(2);
    g.setTextColor(SSD1306_WHITE);
    const int line1Width = 8 * CHAR_W_SIZE2;
    g.setCursor(centerX(line1Width), 6);
    g.print("P");
    g.print(winner);
    g.print(" WINS");

    g.setTextSize(3);
    const int line2Width = 5 * CHAR_W_SIZE3;
    g.setCursor(centerX(line2Width), 32);
    g.print(winsP1);
    g.print(" - ");
    g.print(winsP2);
  }

  bool boxesOverlap(int ax, int ay, int aw, int ah,
                    int bx, int by, int bw, int bh) {
    return !(ax + aw <= bx || bx + bw <= ax ||
             ay + ah <= by || by + bh <= ay);
  }

  void checkAttack(Fighter& attacker, Fighter& victim) {
    if (!attacker.isAttacking()) return;
    int ax, ay, aw, ah; attacker.getAttackHitbox(ax, ay, aw, ah);
    int bx, by, bw, bh; victim.getBodyHitbox(bx, by, bw, bh);
    if (boxesOverlap(ax, ay, aw, ah, bx, by, bw, bh)) {
      int dmg = (ah >= 8) ? 15 : 8;
      victim.takeDamage(dmg);
    }
  }

  void resetRound() {
    delete p1; delete p2;
    p1 = new Fighter(START_X_P1, true);
    p2 = new Fighter(START_X_P2, false);
    koActive = false;
    koTimer  = 0;
    winner   = 0;
  }
}

namespace Arena {

  void init() {
    winsP1 = 0;
    winsP2 = 0;
    resetRound();
  }

  void onAction(int player, Fighter::Action a) {
    if (koActive) return;
    if (player == 1 && p1) p1->doAction(a);
    if (player == 2 && p2) p2->doAction(a);
  }

  void tick(int dtMs) {
    if (!koActive) {
      p1->faceTowards(p2->getX());
      p2->faceTowards(p1->getX());

      p1->tick(dtMs);
      p2->tick(dtMs);

      checkAttack(*p1, *p2);
      checkAttack(*p2, *p1);

      if (!p1->isAlive() || !p2->isAlive()) {
        koActive = true;
        koTimer  = RESTART_MS;
        winner   = p1->isAlive() ? 1 : 2;
        if (winner == 1) winsP1++; else winsP2++;
      }
    } else {
      koTimer -= dtMs;
      if (koTimer <= 0) resetRound();
    }

    Display::beginFrame();
    if (koActive) {
      drawKoBanner();
    } else {
      drawHUD();
      drawFloor();
    }
    Display::endFrame();
  }

}
