#include "dial_device.h"

#include "Arduino.h"

DialDevice::DialDevice(const Hardware& hardware) : hardware(hardware) {
  if (not empty(hardware)) {
    listen(clock_fn);
    listen(data_fn);
    listen(switch_fn);
  }
  clock = true;   // assume the recent past was boring.
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

bool DialDevice::rising() {
  bool was = clock;
  clock = clock_pin();
  return clock and not was;
}

// a request can land mid-copy; hand the whole reading over at once.
static void hand_over(Reading& to, const Reading& from) {
  noInterrupts();
  to = from;
  interrupts();
}

void DialDevice::take(Reading& reading) {
  Reading next = reading;
  bool down = switch_pressed();

  if (rising()) {
    int step = data_pin() ? -1 : 1;
    if (down) {
      next.down_count += step;
    } else {
      next.count += step;
    }
  }
  next.button = down;

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
