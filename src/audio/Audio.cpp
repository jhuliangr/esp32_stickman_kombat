#include "audio/Audio.h"

#include <Arduino.h>

namespace {
  constexpr uint8_t  BUZZER_PIN     = 17;
  constexpr uint8_t  LEDC_CHANNEL   = 1;  
  constexpr uint8_t  LEDC_RES_BITS  = 8;
  constexpr uint32_t LEDC_BASE_FREQ = 2000;

  struct Tone { uint16_t freq; uint16_t durationMs; };

  const Tone SFX_HIT[]   = { { 900, 25 }, { 450, 60 } };
  const Tone SFX_BLOCK[] = { { 220, 40 }, { 160, 50 } };
  const Tone SFX_KO[]    = { { 660, 90 }, { 520, 90 }, { 400, 90 }, { 280, 220 } };
  const Tone SFX_START[] = { { 523, 80 }, { 659, 80 }, { 784, 140 } };

  const Tone*   seq      = nullptr;
  size_t        seqLen   = 0;
  size_t        seqIdx   = 0;
  unsigned long noteStartMs = 0;
  bool          playing  = false;

  void writeFreq(uint16_t hz) {
    if (hz == 0) ledcWriteTone(LEDC_CHANNEL, 0);
    else         ledcWriteTone(LEDC_CHANNEL, hz);
  }

  template <size_t N>
  void start(const Tone (&sfx)[N]) {
    seq         = sfx;
    seqLen      = N;
    seqIdx      = 0;
    noteStartMs = millis();
    playing     = true;
    writeFreq(sfx[0].freq);
  }
}

namespace Audio {

  void init() {
    ledcSetup(LEDC_CHANNEL, LEDC_BASE_FREQ, LEDC_RES_BITS);
    ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);
    writeFreq(0);
  }

  void tick(unsigned long now) {
    if (!playing) return;
    if (now - noteStartMs < seq[seqIdx].durationMs) return;

    seqIdx++;
    if (seqIdx >= seqLen) {
      writeFreq(0);
      playing = false;
      return;
    }
    noteStartMs = now;
    writeFreq(seq[seqIdx].freq);
  }

  void playHit()   { start(SFX_HIT); }
  void playBlock() { start(SFX_BLOCK); }
  void playKO()    { start(SFX_KO); }
  void playStart() { start(SFX_START); }

}
