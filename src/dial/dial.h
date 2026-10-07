#pragma once

#include "data.h"
#include "timing.h"

class DialDevice;

class Dial {
public:
  Dial(const Hardware& hardware);
  Dial(int channel);

  void attach(DialDevice* device);

  void set_channel(int channel) {
    change.channel = channel;
    rhythm.group = channel;
  }

  int channel() {
    return change.channel;
  }

  Reading& update();
  bool press();
  bool release();

  int value() const {
    return reading.count - zero_offset;
  }

  int down_value() const {
    return reading.down_count - down_offset;
  }

  bool button() const {
    return reading.button;
  }

  void zero() {
    zero_offset = reading.count;
    down_offset = reading.down_count;
    change.buffer = (char*)&reading;
    change.to_read = (int)sizeof(Reading);
  }

  Reading reading;

  static u_small active_value(Reading reading);
  u_small active_value();

  void set_clock(Function fn);
  void set_data(Function fn);
  void set_switch(Function fn);
  void set_dial_2();

private:
  int call();
  static int call(Change& change);

  int zero_offset = 0;
  int down_offset = 0;

  bool press_ready = false;
  bool release_ready = false;

  DialDevice* device = nullptr;
  Rhythm rhythm;
  Change change;
};
