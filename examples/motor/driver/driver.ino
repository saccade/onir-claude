#include "Wire.h"

#include "channel.h"
#include "circuits.h"
#include "motor/driver.h"
#include "display/readout.h"
#include "log.h"

Driver* driver{};
Readout* readout{};

Hardware hardware = {};
Instruction instruction{};
const int channel = number(Channel::car);

void take_instruction(int message_size) {
  Wire.readBytes((byte*)&instruction, sizeof(Instruction));
}

void send_outcome() {
  Wire.write((const byte*)&instruction, sizeof(Instruction));
}

void setup() {
  Serial.begin(9600);
  explain = true;  // say each step out loud; false runs smooth and silent
  uno_driver_test(hardware);
  driver = new Driver(hardware);
  readout = new Readout(&instruction, hardware);
  memcheck();

  Serial.print("driver channel ");
  Serial.println(channel);

  Wire.begin(channel);
  Wire.onReceive(take_instruction);
  Wire.onRequest(send_outcome);
}

void loop() {
  if (instruction) {
    driver->follow(instruction);
    if (explain) print_instruction(instruction);
  } else {
    driver->drive();
  }
  readout->refresh();
}
