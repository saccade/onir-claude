#pragma once

#include "data.h"

class DialDevice {
public:
  DialDevice(const Hardware& hardware);
  void take(Reading& reading);

  void set_clock(Function fn);
  void set_data(Function fn);
  void set_switch(Function fn);

  void feel();  // look at the pins and count; pin-change interrupts call this

private:
  // policy
  unsigned settle = 5; // ms the switch must hold still before we believe it

  int spot = 0;        // quarter the shaft sits in; 0 is the detent
  int travel = 0;      // quarters turned since it last sat in a detent

  bool heard = false;  // what the switch said last
  unsigned long heard_at = 0;  // when it started saying it
  bool down = false;   // what we believe

  // counted since the last take(); interrupts add, take() empties
  volatile int notches = 0;
  volatile int notches_down = 0;

  void listen(Function fn);  // pull up a pin and ask to hear when it changes
  int turned();              // +1, -1 or 0, each time it falls into a detent
  bool held();               // the switch, once it stops chattering

  bool clock_pin() const;
  bool data_pin() const;
  bool switch_pressed() const;

  Function clock_fn = Function::clock;
  Function data_fn = Function::data;
  Function switch_fn = Function::switch_;

  const Hardware& hardware;
};
