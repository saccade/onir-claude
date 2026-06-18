// Binds Wire callbacks to addresses in IODevice::state.

#include "Wire.h"

#include "io_device.h"
#include "hardware.h"
#include "circuits.h"
#include "selector.h"
#include "log.h"

DisplayDevice* device;
Display* display;
Hardware hardware = {};

IODevice* io;

int channel = 8;

void on_receive(int n_bytes) {
  int n = n_bytes;
  if (n > (int)sizeof(Message)) {
    Serial.println("Format error!");
    n = (int)sizeof(Message);
  }
  Wire.readBytes((byte*)&io->buffer.display, n);
}

void on_request() {
  Wire.write((byte*)&io->buffer.dial, sizeof(Reading));
}

void nothing() {}

void start() {
  Wire.begin(channel);
  Wire.onReceive(on_receive);
  Wire.onRequest(on_request);
}

void restart(bool button = false) {
  Wire.onReceive(nothing);
  Wire.onRequest(nothing);
  Wire.end();
  blank(&io->buffer.display);
  channel = Selector(button, hardware).get_channel();
  Serial.print("selected: ");
  Serial.println(channel);
  start();
}

void setup() {
  Serial.begin(9600);
  log_winks = 10;  // I need a second.
  Serial.println("starting io device");
  uno_io(hardware);

  io = new IODevice(hardware);
  start();
}

void loop() {
  io->update();
  log(io->buffer);
  if (io->new_channel) {
    io->new_channel = false;
    restart(true);
  }
}
