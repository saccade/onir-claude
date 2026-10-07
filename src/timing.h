#pragma once

#include "data.h"

struct Change {
  int channel = UNSET;
  char* buffer = 0;
  int to_read = 0;
  int to_write = 0;
};

struct Rhythm {
  // present
  long now = UNSET;

  // past
  long last = UNSET;
  int missed = 0;

  // id
  int group = UNSET;

  // policy
  int beats = 5;       // take five per measure
  int value = 10;      // hang in with the beat
  int calls = 3;       // give them a few tries
  int layoff = 1000;   // hold up just a second

};

inline int entrance(const Rhythm& rhythm) {
  return rhythm.group % rhythm.beats;  // sign-preserving modulo
}

inline int stick(const Rhythm& rhythm) {
  if (rhythm.now == UNSET) {  // (is_unset() takes an int; this is a long)
    return UNSET;
  }
  return (unsigned long)rhythm.now % rhythm.beats;  // the clock rolls over; the beat doesn't
}

inline bool on_beat(const Rhythm& rhythm) {
  int beat = stick(rhythm);
  if (beat < 0) { // I can't tell you to go if you don't give me the time.
    return false;
  }
  return entrance(rhythm) == beat;
}

inline bool fresh(const Rhythm& rhythm) {
  return rhythm.last == UNSET;
}

inline bool steady(const Rhythm& rhythm) {
  return rhythm.missed <= rhythm.calls;
}

inline unsigned long since(const Rhythm& rhythm) {
  if (fresh(rhythm)) {
    return rhythm.now;
  }
  return (unsigned long)rhythm.now - (unsigned long)rhythm.last;  // safe across rollover
}

inline unsigned long wait(const Rhythm& rhythm) {
  return steady(rhythm) ? rhythm.value : rhythm.layoff;
}

inline bool on_cue(const Rhythm& rhythm) {
  return fresh(rhythm) or since(rhythm) >= wait(rhythm);
}

inline bool late(const Rhythm& rhythm) {  // slept through our entrance? come in anyway.
  return since(rhythm) >= wait(rhythm) + rhythm.beats;
}

inline bool go(const Rhythm& rhythm) {
  return on_cue(rhythm) and (on_beat(rhythm) or late(rhythm));
}

inline void tally(Rhythm& rhythm, bool hit) {
  if (hit) {
    rhythm.missed = 0;
  } else if (steady(rhythm)) {
    rhythm.missed++;  // once we've laid off, stop counting
  }
}

void keep(Rhythm& rhythm);

using Move = int (*)();
using Call = int (*)(Change& change);


inline int follow(Rhythm& rhythm, Call call, Change& change)  {
  keep(rhythm);
  if (not go(rhythm)) {
    return 0;
  }
  rhythm.last = rhythm.now;

  int response = call(change);
  tally(rhythm, response > 0);
  return response;
}

inline int follow(Rhythm& rhythm, Move move) {
  keep(rhythm);
  if (not go(rhythm)) {
    return 0;
  }
  rhythm.last = rhythm.now;

  int result = move();
  tally(rhythm, result);
  return result;
}
