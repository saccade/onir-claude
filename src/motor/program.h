#pragma once
#include "timing.h"
#include "motor/operation.h"

class Program {
public:

  Instruction instruction;  // client writes here by Wire.
  operator bool() {
    return instruction;
  }

  Operation*& operator[](Cue cue) {
    return operations[(int)cue];
  }
  Operation* operations[(int)Cue::count] = { };
};

template <typename T>
using Execute = int (*)(Program& program, Resource<T>& resource);

template <typename T>
static int follow(Rhythm& rhythm, Execute<T> execute, Program& program, Resource<T>& resource) {
  keep(rhythm);
  if (not go(rhythm)) {
    return 0;
  }
  rhythm.last = rhythm.now;

  int result = execute(program, resource);
  tally(rhythm, result);
  return result;
}
