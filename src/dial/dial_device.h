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
  bool clock = true;  // this pin is HIGH by default

  void listen(Function fn);  // pull up a pin so we can hear the dial
  bool rising();             // clock just went high?

  bool clock_pin() const;
  bool data_pin() const;
  bool switch_pressed() const;

  Function clock_fn = Function::clock;
  Function data_fn = Function::data;
  Function switch_fn = Function::switch_;

  const Hardware& hardware;
};
