#pragma once

#include "data.h"

class DialDevice {
public:
  DialDevice(const Hardware& hardware);
  void take(Reading& reading);

  void set_clock(Function fn);
  void set_data(Function fn);
  void set_switch(Function fn);

private:
  // policy
  unsigned settle = 5; // ms the switch must hold still before we believe it

  // where the contacts sit: clock is the high bit, data the low bit
  int spot = rest;     // assume the recent past was boring
  int travel = 0;      // quarter-steps since we last sat at rest
  static const int rest = 0b11;

  bool heard = false;  // what the switch said last
  unsigned long heard_at = 0;  // when it started saying it
  bool down = false;   // what we believe

  void listen(Function fn);  // pull up a pin so we can hear the dial
  int turned();              // +1, -1 or 0: a whole notch, each time we come to rest
  bool held();               // the switch, once it stops chattering

  bool clock_pin() const;
  bool data_pin() const;
  bool switch_pressed() const;

  Function clock_fn = Function::clock;
  Function data_fn = Function::data;
  Function switch_fn = Function::switch_;

  const Hardware& hardware;
};
