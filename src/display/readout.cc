#include "readout.h"
#include "util.h"

Readout::Readout(Display* display) :display(display) { }

Readout::Readout(Instruction* instruction, Display* display) :
  instruction(instruction), display(display) { }

Readout::Readout(Instruction* instruction, const Hardware& hardware) :
  instruction(instruction), display(new Display(hardware)) { }

// remove
// remove
// remove
// remove
#include "log.h"
// remove
// remove
// remove
// remove

void Readout::refresh() {
  if (not display) return;

  if (not instruction) {
    display->put_str("null");
  } else if (instruction->motion != prior) {
    prior = instruction->motion;
    print_instruction(*instruction);
    display->put(hex_digit((int)(prior.motor) % 16), 0);
    if (prior.pitch < 0) display->put('-', 1);
    display->put(hex_digit(absv(prior.pitch) / 16), 2);
    display->put(hex_digit(absv(prior.pitch) % 16), 3);
  }
  display->refresh();
}

void Readout::report(Instruction* todo) {
  if (not todo) return;

  instruction = todo;
}
