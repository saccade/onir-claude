#include "control.h"
#include "log.h"
#include "screen.h"
#include "circuits.h"

#include "Wire.h"

// const int count = 6;
// int channels[count] = { 8, 9, 10, 11, 12, 13 };
const int count = 4;
int channels[count] = { 8, 9, 10, 11 };

Control* control;
Screen* screen;
Hardware hardware = {};

const char message[64] = "ko labs -- onir --";

const int step_ms = 1000;  // pan one place a second
int step = UNSET;          // the step we're on

void setup() {
  Serial.begin(9600);
  Serial.println("start");
  log_winks = 25;
  uno_io(hardware);
  control = new Control(channels, count, hardware);
  screen = new Screen(control);
  screen->display(message);
  Wire.begin();
}

int now() {
  return millis() / step_ms;
}

bool go() {
  int past = step;
  step = now();
  return past != step;
}

void act() {
  if (go()) {
    screen->pan(1);
    log_io(control->local());
  }
}

void loop() {
  control->check();
  act();
  screen->show();
}
