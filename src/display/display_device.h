#pragma once

#include "display.h"

using Fn = Function;

// Runs a 7 segment common cathode clock display.
class DisplayDevice {

public:
  DisplayDevice(const Hardware& hw = no_hardware);

  void update(const Message s) {
    message = s;
  }

  void refresh();
  void blank();
  void clear() {
    message.clear();
    message.point = UNSET;
  }

  Message message;

private:

  // Map positions to position/cathode pins.
  Function positions[4];

  // segment/anode pins
  Function segments[7];

  /*
  segment names:

      AAAAA
    FF     BB
    FF     BB
      GGGGG
    EE     CC
    EE     CC PP
      DDDDD   PP
  */

  const unsigned ms_per_digit = 1;
  int position = UNSET;          // the digit lit now
  unsigned long shown_at = 0;    // when it lit

  void set_fn_pin(Function fn, bool val);

  // TODO: rename to fn_high? fn_active? (anode/cathode aware)
  void pin_high(Function fn);
  void pin_low(Function fn);
  void set_point_pin(bool val);


  const Hardware& hardware;
};
