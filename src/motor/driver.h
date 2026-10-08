#pragma once

#include "motor/trimmer.h"
#include "motor/machine.h"

class Driver {
public:

  Driver(Machine&);
  Driver(const Hardware&);

  static Code drive(Machine&, Program&);
  Code follow(Instruction& todo);

  Code drive();
  Code update();

private:

  void init();

  Program program { };
  Machine& machine;
};
