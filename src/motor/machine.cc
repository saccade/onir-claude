#include "machine.h"
#include "trimmer.h"

#include "Arduino.h"
#include "Servo.h"
#include "rest.h"

#define LOG_SERVO_CHANGE false

const int PULSE_NEUTRAL = 1500;

// TODO: add custom per-servo trim (data file?)
static int servo_pulse(s_small pitch) {
  // // bit shift by 7 ==> divide by 2^7.
  // long magnitude = ((long)pitch * (long)PULSE_HALF_SPAN);
  // return PULSE_NEUTRAL + (magnitude >> 7);

  return PULSE_NEUTRAL + pitch * 4;  // (this is fine 4 now.)
}

const int PULSE_MIN = servo_pulse(-127);
const int PULSE_MAX = servo_pulse(127);

const int millis_per_wink = 100;  // a wink is half a blink.
long end_millis(u_small winks) {
  if (winks == 0) return UNSET;
  return millis() + winks * millis_per_wink;
}

bool Joint::write() {
  if (not servo) return false;
  servo->write(pulse_usec);
  return true;
}

Code Joint::drive(Instruction& todo) {
  return trimmer->execute(todo);
}

Machine::Machine(const Hardware& hardware) : hardware(hardware) { }

Joint* Machine::engage(Function function, Target target, s_small pitch) {
  Joint* joint = joints[function];
  if (joint) return joint;
  if (not is_motor(function)) return 0;
  joint = new Joint();
  joint->servo = new Servo;
  joint->target = target;
  joint->servo->attach(dispatch(hardware, function));

  joint->pulse_usec = servo_pulse(pitch);
  joint->target_usec = servo_pulse(pitch);
  joint->rhythm.group = int(function);
  joint->rhythm.layoff = 100;  // quick as a wink

  joint->max_delta = 3;

  joints[function] = joint;
  joint->write();
}

void Machine::engage_hardware(Target target) {
  for (Function motor = Function::motor_main; motor < Function::motor_end; motor++) {
    if (dispatch(hardware, motor) != UNSET) {
      Serial.print("engage motor ");
      Serial.println((int)motor);
      engage(motor, target, 0);
    }
  }
}

void Machine::release(Function function) {
  Joint* joint = joints[function];
  if (joint) {
    Servo* servo = joints[function]->servo;
    joint->servo = 0;
    joints[function] = 0;
    servo->detach();
    delete servo;
    delete joint;
  }
}

static Code control(Joint* joint, Motion motion) {
  if (not joint) return Code::missing;

  joint->target_usec = servo_pulse(motion.pitch);
  if (LOG_SERVO_CHANGE) {
    Serial.print("joint->target_usec: ");
    Serial.println(joint->target_usec);
  }
  joint->end_millis = end_millis(motion.winks);
  return Code::perform;
}

void Machine::answer(Function* answer, Function query, Function update) {
  if (*answer == Function::none or update == query)
    *answer = update;  // query motion or first activated
}

Code Machine::assign(const Motion* motion) {
  return motion ? assign(*motion) : Code::reject;
}

Code Machine::assign(const Motion& motion) {
  return control(joints[motion.motor], motion);
}

Function Machine::assign(Operation& operation) {
  if (operation.direction != Cue::query) operation.extend();
  Function query = operation.motion.motor;
  Function response {};
  for (const Motion* motion : operation.motions) {
    Code result = assign(motion);
    bool outcome = performative(result);
    if (true) {
      answer(&response, query, motion->motor);
    }
  }
  return response;
}

int Machine::advance(Function function) {
  Joint* joint = joints[function];
  if (not joint) return 0;
  const int delta = pulse_delta(*joint);
  if (not delta) return 0;

  joint->pulse_usec += delta;
  joint->servo->write(joint->pulse_usec);
  return delta;
}

// a motor is moving if it hasn't reached its target, or it's a wheel that isn't stopped.
static bool moving(const Joint* joint) {
  if (not joint) return false;
  if (joint->pulse_usec != joint->target_usec) return true;
  return joint->target == Target::rotation and joint->pulse_usec != PULSE_NEUTRAL;
}

void Machine::advance() {
  for (Function fn = Function::motor_main; fn < Function::motor_end; fn++) {
    advance(fn);
    if (moving(joints[fn])) stir();
  }
  doze();
}

int Machine::slam(Function function) {
  Joint* joint = joints[function];
  if (not joint) return 0;

  const int delta =  - joint->pulse_usec;
  if (not delta) return 0;

  joint->pulse_usec = joint->target_usec;
  joint->servo->write(joint->pulse_usec);
  return delta;
}

void Machine::halt() {
  for (Function fn = Function::motor_main; fn < Function::motor_end; fn++) {
    halt(fn);
  }
}

static bool stop_seek(Joint* joint) {
  if (not joint) return false;

  if (joint->target != Target::position) return false;

  joint->target_usec = joint->pulse_usec;  // try to stop here
  return true;
}

static bool stop_spin(Joint* joint) {
  if (not joint) return false;
  if (joint->target != Target::rotation) return false;
  joint->target_usec = PULSE_NEUTRAL;  // try to spin down
  return true;
}

static bool hold(Joint* joint) {

  if (not joint->servo) return false;
  if (joint->target == Target::position) return stop_seek(joint);
  if (joint->target == Target::rotation) return stop_spin(joint);
}

void Machine::halt(Function fn) {
  Joint* joint = joints[fn];
  if (joint) {
    hold(joint);
  }
}
