#include "Arduino.h"
#include "Wire.h"

#include "control.h"
#include "display/readout.h"
#include "log.h"

void Control::init() {
  for (Function fn = Function::motor_main; fn < Function::motor_end; fn++) {
    Link* link = panel[fn];
    if (link) {
      if (fn == Function::motor_r_wheel) {
        link->dial->set_dial_2();
      }
      Instruction& todo = link->instruction;
      todo.channel = number(channel);
      todo.command = Code::modify;
      todo.cue = Cue::drive;
      todo.reading.count = UNSET;  // so the first update says hello, before anyone turns a dial
    }
  }
}

Control::Control(Stem& stem, Display* display) :
  panel(stem), readout(new Readout(display)) {
  init();
}

Control::Control(const Hardware& hardware, Display* display) :
  panel(hardware), readout(new Readout(display)) {
  init();
}

void Control::address(Channel channel) {
  channel = channel;
}

static void clear_device(Channel channel) {
  const static Instruction blank { };
  Wire.beginTransmission(number(channel));
  Wire.write((char*)&blank, sizeof(Instruction));
  Wire.endTransmission(number(channel));
}

// num bytes if response; false for missing data or bad response.
static int read_instruction(Channel channel, Instruction* buffer) {
  if (not buffer) return 0;

  Wire.requestFrom(number(channel), sizeof(Instruction));
  if (Wire.available() == sizeof(Instruction)) {
    Wire.readBytes((char*)buffer, sizeof(Instruction));
    if (explain) {
      print_instruction(*buffer);
    }
    return sizeof(Instruction);

  } else if (Wire.available() == 0) {
    Serial.print("no driver. m: ");
    Serial.println(millis());
    return 0;

  } else {
    Serial.print("bad driver! m: ");
    Serial.println(millis());
    return 0;
  }
}

static bool update_panel(const Instruction& incoming, Panel& panel) {
  Link* link = panel[incoming.motion.motor];
  if (not link) return false;
  link->instruction = incoming;
  return true;
}

// true if good response; false for missing or failed response.
bool Control::debrief_driver() {
  static Instruction todo { };
  bool done = false;
  while (not done) {
    int response_size = read_instruction(channel, &todo);
    if (response_size == 0) {
      return false;       // nobody answered.
    }
    if (response_size == sizeof(Instruction)) {
      if (empty(todo)) {
        done = true;      // nothing there.

      } else if (todo) {  // still working
        delay(2);         // this should be quick -- try again.
      } else {
        done = true;      // got it.
      }
    }
  }

  if (empty(todo)) {
    return true;          // nothing to report yet.
  }
  const bool saved = update_panel(todo, panel);
  clear_device(channel);
  return saved and completed(todo);
}

bool Control::update_link(Link* link) {
  if (not link) return false;
  link->dial->update();
  Instruction& todo = link->instruction;
  if (todo.reading != link->dial->reading) {
    if (not debrief_driver()) {
      Serial.println("driver error?");
    }
    todo.reading = link->dial->reading;
    if (todo.command == Code::none) {
      todo.command = Code::perform;
    }
    todo.respond = Code::none;
    Wire.beginTransmission(number(channel));
    Wire.write((char*)&todo, sizeof(Instruction));
    Wire.endTransmission(number(channel));
    update_panel(todo, panel);
    readout->report(&todo);
    return true;
  }
  return false;
}

bool Control::update() {
  bool updated = false;
  for (Function fn = Function::motor_main; fn < Function::motor_end; fn++) {
    updated = update_link(panel[fn]) or updated;
  }
  readout->refresh();
  return updated;
}
