#include "power/Power.h"
#include "display/Display.h"
#include "net/Portal.h"
#include "game/Game.h"

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

namespace {
  constexpr gpio_num_t BUTTON_PIN         = GPIO_NUM_33;
  constexpr int        DEBOUNCE_MS        = 30;
  constexpr int        CONFIRM_TIMEOUT_MS = 3000;

  bool          confirming        = false;
  unsigned long confirmStartedMs  = 0;

  int           lastReading       = HIGH;
  unsigned long lastChangeMs      = 0;
  bool          pressHandled      = false;

  void showSplash() {
    Display::showText("Connect to:", Portal::SSID);
  }

  void enterDeepSleep() {
    Display::showText("Powering", "off...");
    delay(400);
    Display::sleep();

    // Don't re-trigger our own wake source: wait for the button to be
    // released, then arm ext0 to wake on the next press (LOW level).
    while (digitalRead(BUTTON_PIN) == LOW) delay(10);
    delay(50);

    esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0);
    rtc_gpio_pullup_en(BUTTON_PIN);
    rtc_gpio_pulldown_dis(BUTTON_PIN);
    esp_deep_sleep_start();
  }

  // `now` comes from tick() so the confirm timer compares two timestamps
  // captured against the same clock — using millis() inline here would
  // produce a value newer than `now`, making the unsigned diff underflow.
  void onPress(unsigned long now) {
    if (Game::isInCombat()) {
      Game::reset();
      confirming = false;
      return;
    }
    if (confirming) {
      enterDeepSleep();
      return;
    }
    confirming        = true;
    confirmStartedMs  = now;
    Display::showText("Press again", "to power off");
  }
}

namespace Power {

  void init() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    lastReading  = digitalRead(BUTTON_PIN);
    lastChangeMs = millis();
    pressHandled = (lastReading == LOW);
  }

  void tick(unsigned long now) {
    int reading = digitalRead(BUTTON_PIN);
    if (reading != lastReading) {
      lastChangeMs = now;
      lastReading  = reading;
    }
    if (now - lastChangeMs > DEBOUNCE_MS) {
      if (lastReading == LOW && !pressHandled) {
        pressHandled = true;
        onPress(now);
      } else if (lastReading == HIGH) {
        pressHandled = false;
      }
    }

    if (confirming && now - confirmStartedMs > CONFIRM_TIMEOUT_MS) {
      confirming = false;
      showSplash();
    }
  }

}
