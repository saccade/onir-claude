#include "Arduino.h"

#include "driver.h"

#include "log.h"

Driver::Driver(Machine& machine) : machine(machine) {
  init();
}

Driver::Driver(const Hardware& hardware) : machine(*(new Machine(hardware))) {
  machine.engage_hardware(Target::rotation);
  init();
}

static bool zero(Joint* joint, Program& program) {
  if (not joint) return false;
  if (joint->trimmer) return true;
  const Instruction& todo = program.instruction;
  if (todo.cue != Cue::drive) return false;
  Operation*& drive = program[Cue::drive];
  if (not drive) {
    drive = new Operation(todo);
  }
  Motion*& motion = (*drive)[todo.motion.motor];
  if (not motion) {
    motion = new Motion(todo.motion);
  }
  joint->trimmer = new Trimmer(todo.reading, motion);
  return true;
}

void Driver::init() {
  program[Cue::drive] = new Operation(Cue::drive);

  Operation* stop = new Operation(Cue::stop);
  Operation* go = new Operation(Cue::go);

  program[Cue::stop] = stop;
  program[Cue::go] = go;

  for (Function motor = Function::motor_main; motor < Function::motor_end; motor++) {
    Motion* halt =  new Motion();
    halt->motor = motor;
    (*stop)[motor] = halt;

    Motion* move =  new Motion();
    move->motor = motor;
    move->pitch = 1 << 5;
    (*go)[motor] = move;
  }
}

Code Driver::drive(Machine& machine, Program& program) {
  Instruction& todo = program.instruction;
  if (not todo) {
    return sign(todo);                            // nothing to do.
  }

  const Motion& motion = todo.motion;
  Function function = motion.motor;
  if (motion and not is_motor(function)){
    Serial.println("reject 1 ");
    return reject(todo);                          // nonsense
  }

  Joint* joint = machine[function];
  if (not joint) {
    Serial.println("reject 2 ");
    print_instruction(todo);
    return missing(todo);                         // don't have one of those
  }

  Code& command = todo.command;
  Cue cue = todo.cue;

  if (command == Code::perform or command == Code::modify) {
    if (cue == Cue::none) {
      if (not motion) {
        return reject(todo);                    // nothing to perform/modify
      }

      return mark(todo, machine.assign(motion));              // do the motion
    }

    if (cue == Cue::drive) {                                  // manual mode
      if (not motion) {
        return reject(todo);                    // can't drive nothing
      }

      if (not joint->trimmer) zero(joint, program);           // set up trimmer

      if (performative(joint->drive(todo)) and command == Code::perform) {
        return sign(todo, machine.assign(todo.motion));       // activate joint
      }

      return sign_modified(todo);                             // updated drive motion
    }

    Operation* operation_ = program[cue];               // preprogrammed operation
    if (not operation_) {
      if (not motion) {
        return reject(todo);                            // don't know how to do nothing
      } else {
        operation_ = new Operation(todo);
      }
    }

    Operation& operation = *operation_;
    Function motor = machine.assign(operation);   // queried or first found
    todo.motion = *operation[motor];
    todo.direction = operation.direction;
    todo.reading = operation.reading;
    todo.message = operation.message;
    return sign(todo);
  }

  if (command == Code::copy) {
    Cue direction = todo.direction;
    todo.direction = Cue::none;
    program[direction] = new Operation(todo);
  }

  return error(todo);
}

Code Driver::follow(Instruction& todo) {
  program.instruction = todo;
  Code result = drive();
  todo = program.instruction;
  return result;
}

Code Driver::drive() {
  Code response { };
  if (program) {
    drive(machine, program);
  }
  machine.advance();
  if (not program) return release(program.instruction, response);
  return sign(program.instruction, response);
}
