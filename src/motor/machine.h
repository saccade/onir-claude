#pragma once

#include "motor/program.h"

const extern int PULSE_NEUTRAL;
const extern int PULSE_MAX;
const extern int PULSE_MIN;

long end_millis(long duration);

enum class Target : u_small {
			     position,
			     rotation,
			     torque
};

class Trimmer;
class Servo;

struct Joint {
  int pulse_usec = UNSET;
  int target_usec = UNSET;
  long end_millis = UNSET;
  u_small max_delta = 8;  // = beats * 1000 * delta usec / milli
  Target target = Target::position;
  Rhythm rhythm;

  Servo* servo = 0;
  operator bool() const {
    return servo;
  }
  bool write();

  Trimmer* trimmer = 0;
  Code drive(Instruction&);
};

class Machine {
public:
  Machine(const Hardware& hardware = no_hardware);

  Joint*& operator[](Function fn) {
    return joints[fn];
  }

  Joint* engage(Function joint, Target, s_small pitch = 0);
  void engage_hardware(Target);

  void release(Function joint);
  Function assign(Operation&);
  Code assign(const Motion*);
  Code assign(const Motion&);

  int advance(Function joint);
  void advance();  // call in loop()

  void halt(Function joint);
  void halt();

  int slam(Function joint);

  Trimmer* trimmer(Function joint);

private:

  Cue active = Cue::stop;

  const Hardware& hardware;
  Resource<Joint*> joints = {};
};
