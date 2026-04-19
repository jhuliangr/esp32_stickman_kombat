#include "game/FighterRenderer.h"
#include "game/Fighter.h"
#include "display/Display.h"

#include <Adafruit_SSD1306.h>

namespace {
  constexpr int FLOOR_Y  = 58;
  constexpr int HEAD_R   = 4;
  constexpr int HEAD_Y   = FLOOR_Y - 30;
  constexpr int WAIST_Y  = FLOOR_Y - 14;
  constexpr int SHLDR_Y  = FLOOR_Y - 22;

  constexpr int PUNCH_REACH = 10;
  constexpr int KICK_REACH  = 14;

  // Lift applied to head/shoulders on the "legs closed" walk frame so the
  // torso visibly stretches as the fighter rises between steps.
  constexpr int WALK_CLOSED_LIFT = -2;

  int idleBobOffset(int idleTicks) {
    constexpr int HALF_PERIOD_MS = 600;
    return ((idleTicks / HALF_PERIOD_MS) & 1) ? -1 : 0;
  }

  // Walk cycles 0..3: bit 0 picks open vs closed frame, bit 1 swaps lead
  // leg across open frames so successive steps don't look identical.
  bool walkIsOpen (int phase) { return (phase & 1) == 0; }
  bool walkLeadFwd(int phase) { return (phase & 2) == 0; }
  bool runFrontFwd(int phase) { return (phase & 1) == 0; }

  void drawLegsIdle(Adafruit_SSD1306& g, int x) {
    g.drawLine(x, WAIST_Y, x - 3, FLOOR_Y, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y, x + 3, FLOOR_Y, SSD1306_WHITE);
  }

  void drawLegsWalkOpen(Adafruit_SSD1306& g, int x, int dir, bool leadFwd) {
    int front = leadFwd ?  5*dir : -3*dir;
    int back  = leadFwd ? -3*dir :  5*dir;
    int frontKneeX = x + front/2;
    int frontKneeY = FLOOR_Y - 5;
    g.drawLine(x, WAIST_Y, frontKneeX, frontKneeY, SSD1306_WHITE);
    g.drawLine(frontKneeX, frontKneeY, x + front, FLOOR_Y, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y, x + back, FLOOR_Y - 1, SSD1306_WHITE);
  }

  void drawLegsWalkClosed(Adafruit_SSD1306& g, int x) {
    g.drawLine(x, WAIST_Y, x - 1, FLOOR_Y, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y, x + 1, FLOOR_Y, SSD1306_WHITE);
  }

  void drawLegsRun(Adafruit_SSD1306& g, int x, int dir, bool frontFwd) {
    if (frontFwd) {
      int kneeX = x + 4*dir;
      int kneeY = WAIST_Y + 2;
      int footX = x + 8*dir;
      int footY = FLOOR_Y - 3;
      g.drawLine(x, WAIST_Y, kneeX, kneeY, SSD1306_WHITE);
      g.drawLine(kneeX, kneeY, footX, footY, SSD1306_WHITE);
      g.drawLine(x, WAIST_Y, x - 6*dir, FLOOR_Y - 4, SSD1306_WHITE);
    } else {
      int kneeX = x - 3*dir;
      int kneeY = WAIST_Y + 4;
      g.drawLine(x, WAIST_Y, kneeX, kneeY, SSD1306_WHITE);
      g.drawLine(kneeX, kneeY, x + 2*dir, FLOOR_Y, SSD1306_WHITE);
      g.drawLine(x, WAIST_Y, x - 4*dir, WAIST_Y - 2, SSD1306_WHITE);
      g.drawLine(x - 4*dir, WAIST_Y - 2, x - 1*dir, WAIST_Y + 3, SSD1306_WHITE);
    }
  }

  void drawLegsKickWindup(Adafruit_SSD1306& g, int x, int dir) {
    g.drawLine(x, WAIST_Y, x + 3*dir, WAIST_Y - 4, SSD1306_WHITE);
    g.drawLine(x + 3*dir, WAIST_Y - 4, x + 1*dir, WAIST_Y - 1, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y, x - 3*dir, FLOOR_Y, SSD1306_WHITE);
  }
  void drawLegsKickActive(Adafruit_SSD1306& g, int x, int dir) {
    int footX = x + KICK_REACH * dir;
    int footY = HEAD_Y - 4;
    int kneeX = (x + footX) / 2 + 1*dir;
    int kneeY = (WAIST_Y + footY) / 2;
    g.drawLine(x, WAIST_Y, kneeX, kneeY, SSD1306_WHITE);
    g.drawLine(kneeX, kneeY, footX, footY, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y, x + 1*dir, FLOOR_Y, SSD1306_WHITE);
  }
  void drawLegsKickRecover(Adafruit_SSD1306& g, int x, int dir) {
    g.drawLine(x, WAIST_Y, x + 5*dir, WAIST_Y - 2, SSD1306_WHITE);
    g.drawLine(x + 5*dir, WAIST_Y - 2, x + 6*dir, WAIST_Y + 4, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y, x - 3*dir, FLOOR_Y, SSD1306_WHITE);
  }

  void drawArmsIdle(Adafruit_SSD1306& g, int x, int shoulderY) {
    g.drawLine(x, shoulderY, x - 4, shoulderY + 6, SSD1306_WHITE);
    g.drawLine(x, shoulderY, x + 4, shoulderY + 6, SSD1306_WHITE);
  }

  void drawArmsWalkOpen(Adafruit_SSD1306& g, int x, int dir, bool leadFwd) {
    int front = leadFwd ? -3*dir :  3*dir;
    int back  = leadFwd ?  3*dir : -3*dir;
    g.drawLine(x, SHLDR_Y, x + front, SHLDR_Y + 6, SSD1306_WHITE);
    g.drawLine(x, SHLDR_Y, x + back,  SHLDR_Y + 5, SSD1306_WHITE);
  }

  void drawArmsRun(Adafruit_SSD1306& g, int x, int dir, bool frontFwd) {
    if (frontFwd) {
      int elbowX = x - 3*dir;
      int elbowY = SHLDR_Y + 2;
      int handX  = x - 1*dir;
      int handY  = SHLDR_Y - 2;
      g.drawLine(x, SHLDR_Y, elbowX, elbowY, SSD1306_WHITE);
      g.drawLine(elbowX, elbowY, handX, handY, SSD1306_WHITE);
      g.drawLine(x, SHLDR_Y, x + 5*dir, SHLDR_Y + 4, SSD1306_WHITE);
    } else {
      int elbowX = x + 3*dir;
      int elbowY = SHLDR_Y + 2;
      int handX  = x + 1*dir;
      int handY  = SHLDR_Y - 2;
      g.drawLine(x, SHLDR_Y, elbowX, elbowY, SSD1306_WHITE);
      g.drawLine(elbowX, elbowY, handX, handY, SSD1306_WHITE);
      g.drawLine(x, SHLDR_Y, x - 5*dir, SHLDR_Y + 4, SSD1306_WHITE);
    }
  }

  void drawArmsPunchWindup(Adafruit_SSD1306& g, int x, int dir) {
    g.drawLine(x, SHLDR_Y, x - 3*dir, SHLDR_Y - 2, SSD1306_WHITE);
    g.drawLine(x, SHLDR_Y, x + 2*dir, SHLDR_Y + 6, SSD1306_WHITE);
  }
  void drawArmsPunchActive(Adafruit_SSD1306& g, int x, int dir) {
    int px = x + PUNCH_REACH * dir;
    g.drawLine(x, SHLDR_Y, px, SHLDR_Y, SSD1306_WHITE);
    g.drawLine(x, SHLDR_Y, x - 2*dir, SHLDR_Y + 6, SSD1306_WHITE);
  }
  void drawArmsPunchRecover(Adafruit_SSD1306& g, int x, int dir) {
    g.drawLine(x, SHLDR_Y, x + 4*dir, SHLDR_Y + 2, SSD1306_WHITE);
    g.drawLine(x, SHLDR_Y, x - 2*dir, SHLDR_Y + 6, SSD1306_WHITE);
  }
  void drawArmsBlock(Adafruit_SSD1306& g, int x, int dir) {
    g.drawLine(x, SHLDR_Y,     x + 5*dir, SHLDR_Y - 2, SSD1306_WHITE);
    g.drawLine(x, SHLDR_Y + 3, x + 5*dir, SHLDR_Y + 1, SSD1306_WHITE);
  }

  // Full-body override: head, torso and limbs all fold into the crouch.
  void drawDuckPose(Adafruit_SSD1306& g, int x, int dir) {
    constexpr int DUCK_DROP = 8;
    const int headY     = HEAD_Y + DUCK_DROP;
    const int shoulderY = SHLDR_Y + DUCK_DROP;

    g.drawCircle(x, headY, HEAD_R, SSD1306_WHITE);
    g.drawLine(x, headY + HEAD_R, x + dir, WAIST_Y, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y,  x - 5, WAIST_Y + 4, SSD1306_WHITE);
    g.drawLine(x - 5, WAIST_Y + 4, x - 4, FLOOR_Y, SSD1306_WHITE);
    g.drawLine(x, WAIST_Y,  x + 5, WAIST_Y + 4, SSD1306_WHITE);
    g.drawLine(x + 5, WAIST_Y + 4, x + 4, FLOOR_Y, SSD1306_WHITE);
    g.drawLine(x, shoulderY, x - 3, WAIST_Y + 2, SSD1306_WHITE);
    g.drawLine(x, shoulderY, x + 3, WAIST_Y + 2, SSD1306_WHITE);
  }
}

namespace FighterRenderer {

  void render(const Fighter& f) {
    Adafruit_SSD1306& g = Display::canvas();

    const int  x     = f.getX();
    const int  dir   = f.isFacingRight() ? 1 : -1;
    const auto state = f.getState();

    if (state == Fighter::State::DUCK) {
      drawDuckPose(g, x, dir);
      return;
    }

    const bool striding = f.getStrideTimer() > 0;
    const bool running  = f.isRunning();
    const bool walking  = striding && !running
                          && state != Fighter::State::PUNCH
                          && state != Fighter::State::KICK;
    const bool isIdleish =
        (state == Fighter::State::IDLE || state == Fighter::State::BLOCK) &&
        !striding;

    const int bob = isIdleish ? idleBobOffset(f.getIdleTicks()) : 0;

    const bool walkClosedFrame = walking && !walkIsOpen(f.getStridePhase());
    const int  walkBob         = walkClosedFrame ? WALK_CLOSED_LIFT : 0;

    const int runLean = running ? 1 : 0;

    // Kick lean: ACTIVE pitches the upper body back so the head drops below
    // the kicking foot; RECOVERY holds a partial lean before settling.
    int kickHeadDx = 0, kickHeadDy = 0;
    int kickShldDx = 0, kickShldDy = 0;
    if (state == Fighter::State::KICK) {
      auto ph = f.currentAttackPhase();
      if (ph == Fighter::Phase::ACTIVE) {
        kickHeadDx = -6 * dir;  kickHeadDy = 10;
        kickShldDx = -4 * dir;  kickShldDy = 6;
      } else if (ph == Fighter::Phase::RECOVERY) {
        kickHeadDx = -3 * dir;  kickHeadDy = 5;
        kickShldDx = -2 * dir;  kickShldDy = 3;
      }
    }

    const int headY     = HEAD_Y  + bob + walkBob + kickHeadDy;
    const int shoulderY = SHLDR_Y + bob + walkBob + kickShldDy;
    const int headX     = x + runLean * dir + kickHeadDx;
    const int shoulderX = x + kickShldDx;

    const int hitDx = (state == Fighter::State::HIT) ? -2 * dir : 0;
    g.drawCircle(headX + hitDx, headY, HEAD_R, SSD1306_WHITE);

    // Two-segment torso so the kick lean actually bends at the shoulder.
    g.drawLine(headX, headY + HEAD_R, shoulderX, shoulderY, SSD1306_WHITE);
    g.drawLine(shoulderX, shoulderY,  x,         WAIST_Y,   SSD1306_WHITE);

    if (state == Fighter::State::KICK) {
      auto ph = f.currentAttackPhase();
      if      (ph == Fighter::Phase::WINDUP)  drawLegsKickWindup (g, x, dir);
      else if (ph == Fighter::Phase::ACTIVE)  drawLegsKickActive (g, x, dir);
      else                                    drawLegsKickRecover(g, x, dir);
    } else if (walking) {
      if (walkIsOpen(f.getStridePhase()))
        drawLegsWalkOpen(g, x, dir, walkLeadFwd(f.getStridePhase()));
      else
        drawLegsWalkClosed(g, x);
    } else if (striding && running) {
      drawLegsRun(g, x, dir, runFrontFwd(f.getStridePhase()));
    } else {
      drawLegsIdle(g, x);
    }

    if (state == Fighter::State::PUNCH) {
      auto ph = f.currentAttackPhase();
      if      (ph == Fighter::Phase::WINDUP)  drawArmsPunchWindup (g, x, dir);
      else if (ph == Fighter::Phase::ACTIVE)  drawArmsPunchActive (g, x, dir);
      else                                    drawArmsPunchRecover(g, x, dir);
    } else if (state == Fighter::State::BLOCK) {
      drawArmsBlock(g, x, dir);
    } else if (walking) {
      if (walkIsOpen(f.getStridePhase()))
        drawArmsWalkOpen(g, x, dir, walkLeadFwd(f.getStridePhase()));
      else
        drawArmsIdle(g, x, shoulderY);
    } else if (striding && running) {
      drawArmsRun(g, x, dir, runFrontFwd(f.getStridePhase()));
    } else {
      drawArmsIdle(g, x, shoulderY);
    }
  }

}
