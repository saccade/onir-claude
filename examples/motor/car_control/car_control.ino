#include "Wire.h"

#include "channel.h"
#include "circuits.h"
#include "motor/control.h"
#include "log.h"

Hardware hardware = {};
Control* control = {};

void setup() {
  Serial.begin(9600);
  explain = true;  // say each step out loud; false runs smooth and silent
  uno_car(hardware);
  Wire.begin();  // client mode
  control = new Control(hardware);
  memcheck();
  delay(100);  // give the car board a moment to pick up the phone before we say hello
}

void loop() {
  control->update();
}
