#pragma once

class Fighter {
public:
  enum class Action {
    LEFT,
    RIGHT,
    RUN_LEFT,
    RUN_RIGHT,
    PUNCH,
    KICK,
    BLOCK,
    DUCK_ON,
    DUCK_OFF
  };

  enum class State {
    IDLE,
    PUNCH,
    KICK,
    BLOCK,
    DUCK,
    HIT
  };

  enum class Phase { WINDUP, ACTIVE, RECOVERY };

  // Exposed so the renderer can derive phase timing without duplicating them.
  static constexpr int PUNCH_WINDUP_MS  = 70;
  static constexpr int PUNCH_ACTIVE_MS  = 80;
  static constexpr int PUNCH_RECOVER_MS = 70;
  static constexpr int PUNCH_MS =
      PUNCH_WINDUP_MS + PUNCH_ACTIVE_MS + PUNCH_RECOVER_MS;

  static constexpr int KICK_WINDUP_MS   = 100;
  static constexpr int KICK_ACTIVE_MS   = 120;
  static constexpr int KICK_RECOVER_MS  = 100;
  static constexpr int KICK_MS =
      KICK_WINDUP_MS + KICK_ACTIVE_MS + KICK_RECOVER_MS;

  static constexpr int STRIDE_MS = 220;

  Fighter(int startX, bool facingRight);

  void doAction(Action a);
  void tick(int dtMs);
  void takeDamage(int dmg);

  // True only during the ACTIVE window of an attack, not the whole animation.
  bool isAttacking() const;

  void getAttackHitbox(int& x, int& y, int& w, int& h) const;
  // Ducking drops the head out of this box so high attacks pass over.
  void getBodyHitbox(int& x, int& y, int& w, int& h) const;

  void faceTowards(int targetX);

  int   getX()           const { return x_; }
  int   getHP()          const { return hp_; }
  bool  isBlocking()     const { return state_ == State::BLOCK; }
  bool  isDucking()      const { return state_ == State::DUCK; }
  bool  isAlive()        const { return hp_ > 0; }
  State getState()       const { return state_; }
  bool  isFacingRight()  const { return facingRight_; }
  int   getStrideTimer() const { return strideTimer_; }
  int   getStridePhase() const { return stridePhase_; }
  bool  isRunning()      const { return strideTimer_ > 0 && runningStride_; }
  int   getIdleTicks()   const { return idleTicks_; }
  Phase currentAttackPhase() const;

private:
  int    x_;
  int    hp_;
  bool   facingRight_;
  State  state_;
  int    stateTimer_;
  int    cooldown_;

  // strideTimer_ > 0 means a stride pose is in effect; stridePhase_ cycles
  // 0..3 across successive steps so animations don't freeze on repeat input.
  int    strideTimer_;
  int    stridePhase_;
  bool   runningStride_;

  int    idleTicks_;
};
