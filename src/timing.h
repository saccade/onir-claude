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
  long last = 0;       // we last went at boot, as far as anyone knows
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
  return rhythm.now % rhythm.beats;
}

inline bool on_beat(const Rhythm& rhythm) {
  int beat = stick(rhythm);
  if (beat < 0) { // I can't tell you to go if you don't give me the time.
    return false;
  }
  return entrance(rhythm) == beat;
}

inline bool steady(const Rhythm& rhythm) {
  return rhythm.missed <= rhythm.calls;
}

inline long since(const Rhythm& rhythm) {
  return rhythm.now - rhythm.last;
}

inline long wait(const Rhythm& rhythm) {
  return steady(rhythm) ? rhythm.value : rhythm.layoff;
}

inline bool on_cue(const Rhythm& rhythm) {
  return since(rhythm) >= wait(rhythm);
}

inline bool late(const Rhythm& rhythm) {  // a whole measure past our cue? come in anyway.
  return since(rhythm) >= wait(rhythm) + rhythm.beats;
}

inline bool go(const Rhythm& rhythm) {
  return on_cue(rhythm) and (on_beat(rhythm) or late(rhythm));
}

inline void tally(Rhythm& rhythm, bool hit) {
  if (hit) {
    rhythm.missed = 0;
  } else {
    rhythm.missed++;
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
