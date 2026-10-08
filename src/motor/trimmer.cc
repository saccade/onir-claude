#include "trimmer.h"
#include "log.h"

#include "Arduino.h"

Trimmer::Trimmer(const Reading& reading, Motion* motion, bool invert) :
  reading(reading), prior(reading), motion(motion), invert(invert) { }

s_small Trimmer::pitch() {
  if (not motion) return 0;
  return motion->pitch;
}

Code Trimmer::execute(Instruction& todo) {
  if (not performative(todo.command)) return error(todo);

  if (not (todo.cue == Cue::drive)) return error(todo);

  if (not (todo.motion.motor == motion->motor)) return error(todo);

  if (todo.direction == Cue::invert) {
    invert = true;
    if (explain) Serial.println("execute: invert");

    return sign(todo, Code::modify);
  } else if (todo.direction == Cue::revert) {
    invert = false;
    if (explain) Serial.println("execute: revert");
    return sign(todo, Code::modify);
  }

  if (read_dial()) {
    todo.motion.pitch = motion->pitch;
    return mark(todo, Code::modify);
  }
  return mark(todo, Code::idle);
}

bool Trimmer::read_dial() {
  if (prior < reading) {
    (invert) ? pitch_up() : pitch_down();
    prior = reading;
    return true;
  } else if (prior > reading) {
    (invert) ? pitch_down() : pitch_up();
    prior = reading;
    return true;
  }
  return false;
}

bool Trimmer::pitch_down() {
  if (explain) Serial.println("Trimmer::pitch_down");
  if (motion->pitch <= (-1 << 7) + gain) {  // -127
    return false;
  }
  motion->pitch -= gain;
  return true;
}

bool Trimmer::pitch_up() {
  if (explain) Serial.println("Trimmer::pitch_up");
  if (motion->pitch >= (1 << 7) - gain) {  // 127
    return false;
  }
  motion->pitch += gain;
  return true;
}
