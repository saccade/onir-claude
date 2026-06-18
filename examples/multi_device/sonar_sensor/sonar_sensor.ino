#include "circuits.h"
#include "display/display.h"

Hardware hardware = { };
Display* display { };

const int PIN_TRIGGER  = A4;
const int PIN_ECHO     = A5;

const int PIN_SWITCH   = A1;
//const int PIN_ECHO_LED = LED_BUILTIN;

// record program state
bool echo_state = false;
long echo_on_micros;

void update_pin_copies() {
  digitalWrite(dispatch(hardware, Function::trigger), digitalRead(PIN_SWITCH));
//  digitalWrite(PIN_ECHO_LED, digitalRead((dispatch(hardware, Function::echo))));
}

void setup() {
  Serial.begin(9600);
  uno_io(hardware);
  display = new Display(hardware);
  display->set_point(2);

  assign(hardware, Fn::trigger, PIN_ECHO, OUTPUT);
  assign(hardware, Fn::echo, PIN_TRIGGER, INPUT);

  pinMode(PIN_SWITCH, INPUT);
//  pinMode(PIN_ECHO_LED, OUTPUT);


  Serial.println("program start.");
}

void loop() {
  update_pin_copies();
  // log changes in ECHO state
  if (echo_state != digitalRead(dispatch(hardware, Function::echo))) {
    echo_state = !echo_state;
    long micros_now = micros();
    if (echo_state) {
      echo_on_micros = micros();
      Serial.print("on: ");
      Serial.println(micros());
    } else {
      Serial.print("off: ");
      Serial.println(micros());
      long delta_micros = micros() - echo_on_micros;
      long delta_tenths = round(delta_micros / 100.0);
      Serial.println(delta_tenths);
      display->set_value(delta_tenths);
    }
  }
  display->refresh();
}
