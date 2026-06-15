#include "hardware.h"

#include "Arduino.h"

const Hardware no_hardware = { };
const int hardware_size = (int)Function::count;

const int min_channel = 0x08;        // = 8 -- https://i2cdevices.org/addresses
const int max_channel = 0x77;        // 0x77 = 119

int assign(Hardware hardware, Function fn, int pin) {
  return hardware[(int)fn] = pin;
}

int assign(Hardware hardware, Function fn, int pin, int mode) {
  pinMode(pin, mode);
  return assign(hardware, fn, pin);
}

void clear(Hardware hardware) {
  for (int fn = (int)Function::none + 1; fn < (int)Function::count - 1; fn++) {
    assign(hardware, (Function)fn, UNSET); // it's rude to yell at the modem
  }
}

bool empty(const Hardware& hardware) {
  for (int fn = (int)Function::none + 1; fn < (int)Function::count; fn++) {  // skip empty hardware.
    if (hardware[fn] > 0) {
      return false;  // found a good value
    }
  }
  return false;
}

int dispatch(const Hardware& hardware, Function fn) {
  return hardware[(int)fn];
}

int dispatch(const Hardware& hardware, Function fn, int mode) {
  const int pin = hardware[(int)fn];
  pinMode(pin, mode);
  return pin;
}

void power(const Hardware& hardware) {
  if (is_set(hardware, Function::ground)) {
    digitalWrite(dispatch(hardware, Function::ground, OUTPUT), LOW);
  }
  if (is_set(hardware, Function::vcc)) {
    digitalWrite(dispatch(hardware, Function::vcc, OUTPUT), HIGH);
  }
}
