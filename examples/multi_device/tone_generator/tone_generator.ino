#include "circuits.h"
#include "dial/dial.h"
#include "display/display.h"

const int speaker_pin = 11;
const int start_hz = 300;
int current_hz = start_hz;

Dial* dial;
Display* display;

int dial_value;
Hardware hardware = {};

void update_frequency() {
  dial_value = dial->value();
  current_hz = start_hz + dial_value;
  display->set_value(current_hz);
  tone(speaker_pin, current_hz);
  Serial.println(dial_value);
}

void setup() {
  Serial.begin(9600);
  uno_audio(hardware);
  dial = new Dial(hardware);
  display = new Display(hardware);
  Serial.println("starting tone generator");
  update_frequency();
}

void loop() {
  dial->update();
  if (dial_value != dial->value()) { update_frequency(); }
  if (dial->press()) {
    Serial.println("zeroing (");
    Serial.println(millis());
    Serial.println(")");
    dial->zero();
  }
  display->refresh();
}