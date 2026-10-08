#include "display.h"
#include "timing.h"

#include "Arduino.h"
#include "Wire.h"

#include "display_device.h"
#include "rest.h"

Display::Display(const Hardware& hardware) {
  device = new DisplayDevice(hardware);
  clear();
}

Display::Display(int channel) {
  rhythm.group = channel;
  rhythm.beats = 11;
  change.channel = channel;
  change.buffer = (char*)&message;
  change.to_write = sizeof(Message);
  clear();
}

int Display::call(Change& change) {
  if (change.channel < min_channel) return 0;
  Wire.beginTransmission(change.channel);
  Wire.write(change.buffer, change.to_write);
  return Wire.endTransmission() == 0;  // 0 = got it. (silence means success)
}

int Display::call() {
  return call(change);
}

void Display::refresh() {
  if (spent()) {  // say goodbye
    put_str("boot");
    clear_point();
  }
  if (showing(message)) stir();
  follow(rhythm, call, change);
  if (device) {
    device->update(message);
    device->refresh();
  }
  doze();
}

String chars(Message message) {
  char result[5];
  for (int i = 0; i < 4; i++) {
    result[i] = (char)message.chars[i];
  }
  result[4] = 0;
  return String(result);
}
