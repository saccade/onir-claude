#include "dial_device.h"

#include "Arduino.h"

DialDevice::DialDevice(const Hardware& hardware) : hardware(hardware) {
  if (not empty(hardware)) {
    listen(clock_fn);
    listen(data_fn);
    listen(switch_fn);
  }
}

void DialDevice::listen(Function fn) {
  dispatch(hardware, fn, INPUT_PULLUP);
}

bool DialDevice::clock_pin() const {
  return digitalRead(dispatch(hardware, clock_fn));
}

bool DialDevice::data_pin() const {
  return digitalRead(dispatch(hardware, data_fn));
}

bool DialDevice::switch_pressed() const {
  return not digitalRead(dispatch(hardware, switch_fn));
}

// one quarter-step between two spots: +1 the turn-up way (clock leads),
// -1 the other way, 0 for standing still or a jump we can't read.
// a chattering contact flips back and forth, and its quarter-steps cancel.
static int nudge(int from, int to) {
  static const s_small steps[16] = {
  //  to: 00  01  10  11
          0, -1,  1,  0,   // from 00
          1,  0,  0, -1,   // from 01
         -1,  0,  0,  1,   // from 10
          0,  1, -1,  0,   // from 11
  };
  return steps[from << 2 | to];
}

int DialDevice::turned() {
  int was = spot;
  spot = clock_pin() << 1 | data_pin();
  travel += nudge(was, spot);

  if (spot != rest) {
    return 0;
  }
  int notch = 0;
  if (travel > 1) notch = 1;     // most of the way round counts
  if (travel < -1) notch = -1;
  travel = 0;
  return notch;
}

bool DialDevice::held() {
  bool now = switch_pressed();
  if (now != heard) {
    heard = now;
    heard_at = millis();
  }
  if (millis() - heard_at >= settle) {
    down = heard;
  }
  return down;
}

// a request can land mid-copy; hand the whole reading over at once.
static void hand_over(Reading& to, const Reading& from) {
  noInterrupts();
  to = from;
  interrupts();
}

void DialDevice::take(Reading& reading) {
  Reading next = reading;
  bool pressed = held();

  int step = turned();
  if (pressed) {
    next.down_count += step;
  } else {
    next.count += step;
  }
  next.button = pressed;

  hand_over(reading, next);
}

void DialDevice::set_clock(Function fn) {
  clock_fn = fn;
  listen(fn);
}

void DialDevice::set_data(Function fn) {
  data_fn = fn;
  listen(fn);
}

void DialDevice::set_switch(Function fn) {
  switch_fn = fn;
  listen(fn);
}
