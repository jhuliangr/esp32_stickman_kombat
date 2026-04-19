#include "game/Fighter.h"
#include "display/Display.h"

namespace {
  constexpr int WALK_STEP_PX = 4;
  constexpr int RUN_STEP_PX  = 8;
  constexpr int X_MIN        = 8;
  constexpr int X_MAX        = Display::WIDTH - 8;

  constexpr int FLOOR_Y  = 58;
  constexpr int HEAD_R   = 4;
  constexpr int HEAD_Y   = FLOOR_Y - 30;
  constexpr int WAIST_Y  = FLOOR_Y - 14;
  constexpr int SHLDR_Y  = FLOOR_Y - 22;

  constexpr int PUNCH_REACH = 10;
  constexpr int KICK_REACH  = 14;

  constexpr int BLOCK_DMG   = 1;

  constexpr int HIT_MS       = 220;
  constexpr int COOLDOWN_MS  = 120;
}

Fighter::Fighter(int startX, bool facingRight)
  : x_(startX),
    hp_(100),
    facingRight_(facingRight),
    state_(State::IDLE),
    stateTimer_(0),
    cooldown_(0),
    strideTimer_(0),
    stridePhase_(0),
    runningStride_(false),
    idleTicks_(0)
{}

namespace {
  bool canActFrom(Fighter::State s) {
    return s == Fighter::State::IDLE || s == Fighter::State::BLOCK;
  }
}

void Fighter::doAction(Action a) {
  if (state_ == State::HIT) return;

  auto doStep = [&](int dx, bool running) {
    if (!canActFrom(state_)) return;
    x_ += dx;
    if (x_ < X_MIN) x_ = X_MIN;
    if (x_ > X_MAX) x_ = X_MAX;
    if (state_ == State::BLOCK) state_ = State::IDLE;
    strideTimer_    = STRIDE_MS;
    stridePhase_    = (stridePhase_ + 1) & 3;
    runningStride_  = running;
  };

  switch (a) {
    case Action::LEFT:       doStep(-WALK_STEP_PX, false); break;
    case Action::RIGHT:      doStep(+WALK_STEP_PX, false); break;
    case Action::RUN_LEFT:   doStep(-RUN_STEP_PX,  true);  break;
    case Action::RUN_RIGHT:  doStep(+RUN_STEP_PX,  true);  break;

    case Action::PUNCH:
      if (cooldown_ <= 0 && canActFrom(state_)) {
        state_       = State::PUNCH;
        stateTimer_  = PUNCH_MS;
        cooldown_    = PUNCH_MS + COOLDOWN_MS;
        strideTimer_ = 0;
      }
      break;

    case Action::KICK:
      if (cooldown_ <= 0 && canActFrom(state_)) {
        state_       = State::KICK;
        stateTimer_  = KICK_MS;
        cooldown_    = KICK_MS + COOLDOWN_MS;
        strideTimer_ = 0;
      }
      break;

    case Action::BLOCK:
      if      (state_ == State::IDLE)  state_ = State::BLOCK;
      else if (state_ == State::BLOCK) state_ = State::IDLE;
      break;

    case Action::DUCK_ON:
      if (canActFrom(state_)) {
        state_       = State::DUCK;
        strideTimer_ = 0;
      }
      break;

    case Action::DUCK_OFF:
      if (state_ == State::DUCK) state_ = State::IDLE;
      break;
  }
}

void Fighter::tick(int dtMs) {
  if (stateTimer_ > 0) {
    stateTimer_ -= dtMs;
    if (stateTimer_ <= 0) {
      stateTimer_ = 0;
      if (state_ != State::BLOCK && state_ != State::DUCK) state_ = State::IDLE;
    }
  }
  if (cooldown_    > 0) cooldown_    -= dtMs;
  if (strideTimer_ > 0) {
    strideTimer_ -= dtMs;
    if (strideTimer_ <= 0) {
      strideTimer_   = 0;
      runningStride_ = false;
    }
  }

  const bool trulyIdle =
      (state_ == State::IDLE || state_ == State::BLOCK) && strideTimer_ <= 0;
  if (trulyIdle) {
    idleTicks_ += dtMs;
    if (idleTicks_ > 60000) idleTicks_ -= 60000;
  } else {
    idleTicks_ = 0;
  }
}

void Fighter::takeDamage(int dmg) {
  if (state_ == State::BLOCK) dmg = BLOCK_DMG;
  hp_ -= dmg;
  if (hp_ < 0) hp_ = 0;
  // Ducking doesn't soak damage here; it dodges via the body hitbox shape.
  if (state_ != State::BLOCK) {
    state_      = State::HIT;
    stateTimer_ = HIT_MS;
  }
}

Fighter::Phase Fighter::currentAttackPhase() const {
  int totalMs  = 0;
  int windupMs = 0;
  int activeMs = 0;
  if (state_ == State::PUNCH) {
    totalMs  = PUNCH_MS;
    windupMs = PUNCH_WINDUP_MS;
    activeMs = PUNCH_ACTIVE_MS;
  } else if (state_ == State::KICK) {
    totalMs  = KICK_MS;
    windupMs = KICK_WINDUP_MS;
    activeMs = KICK_ACTIVE_MS;
  } else {
    return Phase::RECOVERY;
  }
  int elapsed = totalMs - stateTimer_;
  if (elapsed < windupMs)            return Phase::WINDUP;
  if (elapsed < windupMs + activeMs) return Phase::ACTIVE;
  return Phase::RECOVERY;
}

bool Fighter::isAttacking() const {
  if (state_ != State::PUNCH && state_ != State::KICK) return false;
  return currentAttackPhase() == Phase::ACTIVE;
}

void Fighter::getAttackHitbox(int& x, int& y, int& w, int& h) const {
  if (state_ == State::PUNCH) {
    w = 6; h = 6;
    y = SHLDR_Y - 1;
    x = facingRight_ ? (x_ + PUNCH_REACH - w/2) : (x_ - PUNCH_REACH - w/2);
  } else if (state_ == State::KICK) {
    w = 8; h = 8;
    y = HEAD_Y - h/2;
    x = facingRight_ ? (x_ + KICK_REACH - w/2) : (x_ - KICK_REACH - w/2);
  } else {
    x = y = w = h = 0;
  }
}

void Fighter::getBodyHitbox(int& x, int& y, int& w, int& h) const {
  w = 10;
  if (state_ == State::DUCK) {
    y = SHLDR_Y + 4;
    h = FLOOR_Y - (SHLDR_Y + 4);
  } else {
    y = HEAD_Y - HEAD_R;
    h = FLOOR_Y - y;
  }
  x = x_ - w/2;
}

void Fighter::faceTowards(int targetX) {
  facingRight_ = (targetX > x_);
}
