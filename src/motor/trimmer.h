#pragma once

#include "data.h"

class Trimmer {
public:
  Trimmer(const Reading&, Motion* motion, bool invert = false);

  s_small pitch();

  // checks and handles correctly-addressed instructions
  Code execute(Instruction& todo);

private:

  const Reading& reading;
  bool read_dial();  // doesn't check anything, assumes good reading
  Reading prior;     // copy needed to spot changes

  Motion* motion;

  bool invert = false;
  int gain = 2;

  bool pitch_down();
  bool pitch_up();
};
