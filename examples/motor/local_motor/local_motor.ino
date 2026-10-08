#include "circuits.h"

#include "motor/driver.h"
#include "dial/dial.h"

#include "log.h"

Hardware hardware = {};
Driver* driver{};

Instruction drive_l{};
Dial* dial_l{};

Instruction drive_r{};
Dial* dial_r{};

void update_side(Dial* dial, Instruction& instruction) {
  if (dial->update() != instruction.reading) {
    instruction.reading = dial->reading;
    instruction.command = Code::perform;
    instruction.respond = Code::none;
    driver->follow(instruction);
    if (explain) print_instruction(instruction);
  }
}

void setup() {
  Serial.begin(9600);
  explain = true;  // say each step out loud; false runs smooth and silent
  uno_car(hardware);
  driver = new Driver(hardware);
  dial_l = new Dial(hardware);
  dial_r = new Dial(hardware);
  dial_r->set_dial_2();

  // driving instructions
  drive_l.command = Code::perform;
  drive_l.cue = Cue::drive;
  drive_l.reading.count = UNSET;
  drive_r = Instruction(drive_l);

  drive_l.motion.motor = Function::motor_l_wheel;
  drive_r.motion.motor = Function::motor_r_wheel;

  Serial.print("start loop (~");
  Serial.print(gamut<Instruction>());
  Serial.println("B free):");
}

void loop() {
  update_side(dial_l, drive_l);
  update_side(dial_r, drive_r);
  driver->drive();
}
