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

void DialDevice::feel() {
  bool pressed = held();
  int step = turned();
  if (pressed) {
    moved_down += step;
  } else {
    moved += step;
  }
}

void DialDevice::take(Reading& reading) {
  noInterrupts();
  feel();  // catch up on anything no interrupt told us about
  Reading next = reading;
  next.count += moved;
  next.down_count += moved_down;
  next.button = down;
  moved = moved_down = 0;
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
