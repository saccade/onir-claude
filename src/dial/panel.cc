#include "panel.h"

#include "Arduino.h"

#include "log.h"

Link::Link(Dial* dial) : dial(dial) { }

Link::Link(const Hardware& hardware) : dial(new Dial(hardware)) { }

Link::Link(Link* link ) {
  dial = link->dial;
  instruction = Instruction {link->instruction};
}

Panel::Panel(const Hardware& hardware) {
  for (Function fn = Function::motor_main; fn < Function::motor_end; fn++) {
    if (is_set(hardware, fn)) {
      if (explain) Serial.println((int)fn);
      Link*& link = operator[](fn);
      link = new Link(hardware);
      link->instruction.cue = Cue::drive;
      link->instruction.motion.motor = fn;
      if (explain) print_instruction(link->instruction);
    }
  }
}

Stem::Stem(const Hardware& hardware) {
  left()  = new Link(new Dial(hardware));
  right() = new Link(new Dial(hardware));
  right()->dial->set_dial_2();
}

Link*& Stem::left() {
  return dials[(int)fn_left];
}

Link*& Stem::right() {
  return dials[(int)fn_right];
}

Stem::Stem(Dial* l, Dial* r) {
  left() = new Link(l);
  right() = new Link(r);
  r->set_dial_2();
}
