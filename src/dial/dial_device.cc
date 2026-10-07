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

// where the shaft sits: 0 in the detent, then 1, 2, 3 turning up.
static int quarter(bool clock, bool data) {
  if (clock and data) return 0;  // in the detent
  if (data) return 1;            // clock let go first
  if (clock) return 3;           // data let go first
  return 2;                      // both let go
}

// quarters from one spot to the next: up 1, down -1, still or lost 0.
static int moved(int from, int to) {
  int step = (to - from + 4) % 4;
  if (step == 1) return 1;
  if (step == 3) return -1;
  return 0;
}

int DialDevice::turned() {
  int was = spot;
  spot = quarter(clock_pin(), data_pin());
  travel += moved(was, spot);

  if (spot != 0) return 0;  // flip only once it falls in
  int notch = 0;
  if (travel > 1) notch = 1;
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
