#pragma once

#include "motor/program.h"

const extern int PULSE_NEUTRAL;
const extern int PULSE_MAX;
const extern int PULSE_MIN;

// converts motor pitch (signed 8 bit integer) into servo pulse in usec (16 bit)
static int servo_pulse(s_small pitch);

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

static bool stop_seek(Joint* joint);
static bool stop_spin(Joint* joint);
static bool hold(Joint* joint);

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

  static void answer(Function* answer, Function query, Function update);

  static int pulse_delta(const Joint& joint) {
    int target_delta = joint.target_usec - joint.pulse_usec;
    if (not target_delta) return 0;
    int sign = (target_delta > 0) ? 1 : -1;

    if (sign * target_delta <= joint.max_delta) {
      return target_delta;
      } else {
      return sign * joint.max_delta;
    }
  }

  Cue active = Cue::stop;

  const Hardware& hardware;
  Resource<Joint*> joints = {};
};
