#include "Wire.h"

#include "channel.h"
#include "circuits.h"

#include "display/display_device.h"

DisplayDevice* display;
Hardware hardware = { };

void update_display(int message_size) {
  Wire.readBytes((byte*)&display->message, message_size);
}

void setup() {
  Serial.begin(9600);
  Serial.println("starting display.");
  uno_io(hardware);
  display = new DisplayDevice(hardware);

  Wire.begin(number(Channel::display));
  Wire.onReceive(update_display);
}

void loop() {
  display->refresh();
}
