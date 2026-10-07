#include "dial_device.h"

#include "Arduino.h"

// drawers for the dials on this board, so a pin change can find them.
static const int seats = 4;
static DialDevice* dials[seats] = { };

static void sit(DialDevice* dial) {
  for (int i = 0; i < seats; i++) {
    if (not dials[i]) {
      dials[i] = dial;
      return;
    }
  }
}

static void hear_all() {
  for (int i = 0; i < seats; i++) {
    if (dials[i]) dials[i]->feel();
  }
}

ISR(PCINT0_vect) { hear_all(); }  // pins 8-13
ISR(PCINT1_vect) { hear_all(); }  // A0-A5
ISR(PCINT2_vect) { hear_all(); }  // pins 0-7

// ask the chip to interrupt us when this pin changes.
static void watch(int pin) {
  volatile uint8_t* mask = digitalPinToPCMSK(pin);
  if (not mask) return;  // not a pin-change pin; take() still polls it
  *mask |= bit(digitalPinToPCMSKbit(pin));
  PCICR |= bit(digitalPinToPCICRbit(pin));
}

DialDevice::DialDevice(const Hardware& hardware) : hardware(hardware) {
  if (not empty(hardware)) {
    listen(clock_fn);
    listen(data_fn);
    listen(switch_fn);
  }
  sit(this);
}

void DialDevice::listen(Function fn) {
  watch(dispatch(hardware, fn, INPUT_PULLUP));
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

void DialDevice::feel() {
  bool pressed = held();
  int step = turned();
  if (pressed) {
    notches_down += step;
  } else {
    notches += step;
  }
}

void DialDevice::take(Reading& reading) {
  noInterrupts();
  feel();  // catch up on anything no interrupt told us about
  Reading next = reading;
  next.count += notches;
  next.down_count += notches_down;
  next.button = down;
  notches = notches_down = 0;
  interrupts();

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
