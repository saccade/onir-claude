#include "rest.h"

#include "Arduino.h"

static const unsigned long hour = 60UL * 60 * 1000;
static const unsigned long weary = 160 * hour;       // start over once it's quiet
static const unsigned long done = 24 * 24 * hour;    // start over no matter what
static const unsigned long quiet = 60UL * 1000;      // a minute of nothing happening
static const unsigned long notice = 5000;            // how long "boot" shows

static unsigned long stirred = 0;  // when something last happened

void (*ResetSystem)(void);  // calling it jumps to 0, and the program starts over.
                            // (the reset pin causes upload problems.)

void stir() {
  stirred = millis();
}

bool spent() {
  return millis() >= done;
}

void doze() {
  unsigned long now = millis();
  bool tired = now >= weary and now - stirred >= quiet;
  bool over = now >= done + notice;
  if (tired or over) ResetSystem();
}
