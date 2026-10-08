#pragma once

#include "hardware.h"

struct Reading {
  int count = 0;
  int down_count = 0;
  bool button = false;

  bool operator==(const Reading& other) const {
    return (count == other.count and
            down_count == other.down_count and
            button == other.button);

  };

  bool operator!=(const Reading& other) const {
    return not operator==(other);

  };

  bool operator<(const Reading& other) const {
    if (count != other.count) {
      return count < other.count;
    }
    if (down_count != other.down_count) {
      return down_count < other.down_count;
    }
    return false;
  }

  bool operator>(const Reading& other) const {
    if (count != other.count) {
      return count > other.count;
    }
    if (down_count != other.down_count) {
      return down_count > other.down_count;
    }
    return false;
  }
};

struct Message {
  char chars[4] = {0, 0, 0, 0} ;  // characters on display
  s_small point = UNSET;          // values outside [0,3] are pointless.

  void clear() {
    for (int i = 0; i < 4; i++) {
      chars[i] = ' ';
    }
  }

};

struct Motion {  // TODO: remove default ctor; require a motor(function).
  Function motor = Function::none;
  operator bool() const {
    return motor != Function::none;
  }

  s_small pitch = 0;
  u_small winks = 10;  // try for a second

  void clear() {
    motor = Function::none;
    pitch = 0;
    winks = 0;
  }

  bool operator==(const Motion& other) const {
    return motor == other.motor and pitch == other.pitch;  // timeout (winks) may vary
  }
};

enum class Cue : u_small {
  none,   // unset
  stop,   // static responses for motor type ("stay there" or "don't spin")
  go,     // default action

  // act
  forward,
  back,
  spin_clockwise,
  spin_counterwise,
  go_right,
  go_left,
  back_right,
  back_left,

  // follow
  drive,   // set pitch by dial
  scan,    // watch IR sensor

  // modify
  invert,  // flip dial meaning
  revert,  // normal reading

  query,   // ask about motor motion

  count,   // last item used for size
};

inline bool is_motor(Cue cue) {
  int i = (int)cue;
  return (i >= (int)Cue::stop and i <=(int)Cue::scan);
}

enum class Code : u_small {
  none,       //                                              0
  perform,    // showtime                                     1

  modify,     // add or modify motion for cue                 2
  copy,       // duplicate cue (uses .direction)              3
  forget,     // delete object                                4

  idle,       // driver indicates no action                   5
  missing,    // driving indicates lack of requested joint    6
  reject,     // driver indicates bad instruction             7
  error,      // logical exception in device (== layer)       8

  // condition,  // NOT IMPLEMENTED (place condition on cue)  9

};

static inline bool imperative(const Code& command) {  // sensible request?
  if (command == Code::none)    return false;

  if (command == Code::idle)    return false;
  if (command == Code::reject)  return false;
  if (command == Code::missing) return false;
  if (command == Code::error)   return false;

  return true;
}



static inline bool performative(const Code& response) {  // need motor update?
  if (response == Code::perform) return true;
  if (response == Code::modify)  return true;
  return false;
}

static inline bool responsive(const Code& command) {  // active response?
  if (command == Code::none)    return false;

  if (command == Code::idle)    return true;
  if (command == Code::missing) return true;
  if (command == Code::reject)  return true;
  if (command == Code::error)   return true;

  return false;
}

static bool informative(const Code& response) {  // valid response?
  return response != Code::none;
}

struct Interaction;

struct Instruction {
  Code command = Code::none;    // boss to driver
  Code respond = Code::none;    // driver to boss

  // response codes:
  //
  // {(c)ommand; (r)espond}

  // INVALID (but still boolean-false)
  // {c= none;       r = none}      == misformed. (why send this?)

  // TRUE values (there could be something to do):

  // incoming:
  // {c = <active>;  r = none}      == incoming (from client)
  //
  // processing:
  // {c = <active>;  r = reject}    == illogical request (unsigned)
  // {c = <active>;  r = <other>}   == processing (mid-task)

  // FALSE values (nothing to do here):

  // done:
  // {c = none;      r = <active>}  == took an action (expected)

  // apt requests:
  // {c= none;       r = idle}      == no action needed
  // {c= idle;       r = idle}      == no action needed -- rate signal

  // bad requests:
  // {c = reject     r = none}      == no request made
  // {c = reject;    r = idle}      == over-rate error
  // {c = reject     r = missing}   == missing needed hardware
  // {c = reject     r = error}     == bad state -- logic error in device
  // {c = reject;    r = <other>}   == unreasonable request

  // returns true if instruction needs action:
  operator bool() const {
    if (imperative(command) and not informative(respond)) return true;
    return false;
  }

  Cue cue;                    // engage motor cue
  Motion motion;

  Message message;            // displays
  Reading reading;            // sensors

  Instruction* next = 0;      // next intruction; if none, try direction
  Cue direction = Cue::none;
  s_small channel = UNSET;

  void clear() {
    Instruction blank;
    *this = blank;
  }
};

struct Loop {
// same first two elements as Instruction; can be read from driver alone.
  Code command = Code::none;    // boss to driver
  Code respond = Code::none;    // driver to boss

  Loop() { }
  Loop(Instruction instruction) :
    command(instruction.command), respond(instruction.respond) { }

  Instruction& update(Instruction& instruction) {
    instruction.command = command;
    instruction.respond = respond;
    return instruction;
  }
};

static inline bool empty(const Loop& todo) {
  return todo.command == Code::none and todo.respond == Code::none;
}

static inline bool idled(const Loop& todo) {
  return todo.command == Code::none and todo.respond == Code::idle;
}

static inline bool failed(const Loop& todo) {
  if (idled(todo)) return false;
  return responsive(todo.respond);
}

static inline bool rejected(const Loop& todo) {
  return todo.respond == Code::reject;
}

static inline bool performed(const Loop& todo) {
  return todo.command == Code::none and todo.respond == Code::perform;
}

static inline bool modified(const Loop& todo) {
  return todo.command == Code::none and todo.respond == Code::modify;
}

static inline bool copied(const Loop& todo) {
  return todo.command == Code::none and todo.respond == Code::copy;
}

static inline bool forgotten(const Loop& todo) {
  return todo.command == Code::none and todo.respond == Code::forget;
}

static inline bool succeeded(const Loop& todo) {
  return todo.command == Code::none and imperative(todo.respond);
}

static inline bool completed(const Loop& todo) {
  return succeeded(todo) or idled(todo);
}

static inline Instruction& perform(Instruction& todo) {
  todo.command = Code::perform;
  todo.respond = Code::none;
  return todo;
}

static inline Instruction& modify(Instruction& todo) {
  todo.command = Code::modify;
  todo.respond = Code::none;
  return todo;
}

static inline Instruction& copy(Instruction& todo) {
  todo.command = Code::copy;
  todo.respond = Code::none;
  return todo;
}

static inline Instruction& forget(Instruction& todo) {
  todo.command = Code::forget;
  todo.respond = Code::none;
  return todo;
}

static inline Code& sign_modified(Instruction& todo) {
  todo.command = Code::none;
  todo.respond = Code::modify;
  return todo.respond;
}

// sensible request, but no action was needed.
static Code idle(Instruction& todo) {
  todo.respond = Code::idle;
  todo.command = Code::none;  // TODO: set rate-limit flag values here
  return todo.command;
}

// return response value; let go of todo unaltered.
static Code release(Instruction& todo, Code response) {
  return response;
}

// let go of todo unaltered.
static Code& release(Instruction& todo) {
  return todo.respond;
}

// set and return response field from response; release instruction.
static Code& mark(Instruction& todo, Code response) {
  todo.respond = response;
  return release(todo);
}

static Code& apply(Code response, Instruction& todo) {
  return mark(todo, response);
}

static Code& reject(Instruction& todo) {
  todo.respond = Code::reject;
  return todo.respond;
}

static Code& missing(Instruction& todo) {
  todo.respond = Code::missing;
  return todo.respond;
}

static Code& error(Instruction& todo) {
  todo.respond = Code::error;
  return todo.respond;
}

static Code& sign(Instruction& todo) {
  if (not informative(todo.command)) {
    return release(todo);                // nothing to do yet.
  }

  if (not imperative(todo.command)) {
    return reject(todo);                // nonsense
  }

  if (not informative(todo.respond)) {  // logic error -- needed to sign.
    return error(todo);
  }
  todo.command = Code::none;
  return release(todo);
}

static Code sign(Instruction& todo, Code response) {
  todo.respond = response;
  return sign(todo);
}
