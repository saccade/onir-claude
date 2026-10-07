#include "dial.h"
#include "dial_device.h"

#include "util.h"

#include "Wire.h"

Dial::Dial(const Hardware& hardware) {
  device = new DialDevice(hardware);
  zero();
}

Dial::Dial(int channel) {
  zero();
  set_channel(channel);
}

void Dial::attach(DialDevice* local) {
  device = local;
}

int Dial::call(Change& change) {
  if (is_unset(change.channel)) return 0;

  Wire.requestFrom(change.channel, change.to_read);

  if (Wire.available() == change.to_read) {
    Wire.readBytes(change.buffer, change.to_read);
    return change.to_read;
  }
  return 0;
}

int Dial::call() {
  return call(change);
}

Reading& Dial::update() {
  if (device) {
    device->take(reading);
  } else {
    follow(rhythm, call, change);
  }
  return reading;
}

bool Dial::press() {
  if (press_ready and reading.button) {
    press_ready = false;
    return true;
  }
  if (not press_ready and not reading.button) {
    press_ready = true;
  }
  return false;
}

bool Dial::release() {
  if (release_ready and not reading.button) {
    release_ready = false;
    return true;
  }
  if (not release_ready and reading.button) {
    release_ready = true;
  }
  return false;
}

u_small Dial::active_value(Reading reading) {
  if (reading.button) {
    return mod(256, reading.down_count);
  } else {
    return mod(256, reading.count);
  }
}

u_small Dial::active_value() {
  return active_value(reading);
}

void Dial::set_clock(Function fn) {
  device->set_clock(fn);
}

void Dial::set_data(Function fn) {
  device->set_data(fn);
}

void Dial::set_switch(Function fn) {
  device->set_switch(fn);
}

void Dial::set_dial_2() {
  set_clock(Function::clock_2);
  set_data(Function::data_2);
  set_switch(Function::switch_2);
}
