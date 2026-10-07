#include "circuits.h"
#include "display/display.h"
#include "display/display_device.h"
#include "dial/dial.h"
#include "dial/dial_device.h"
#include "selector.h"
#include "onir.h"

#include "Wire.h"

DisplayDevice* device;
Hardware hardware = { };

int channel;

const unsigned int channel_display_ms = 1600;
const unsigned int default_channel_start_ms = 3 * channel_display_ms;

// for setup
int dial_value = 0;
bool channel_display = true;
bool recieved = false;

long start_millis = -1;
bool running() {
  return start_millis >= 0;
}

void clear() {
  if (not channel_display) return;  // already cleared.
  if (recieved) {                   // already in use.
    channel_display = false;
    return;
  }
  if (millis() > start_millis + channel_display_ms) {
    device->clear();
    channel_display = false;
  }
}

void update_display(int message_size) {
  Wire.readBytes((byte*)&(device->message), min(message_size, (int)sizeof(Message)));
  recieved = true;
}

void init_channel(int channel) {
  start_millis = millis();
  Serial.print("channel ");
  Serial.print(channel);
  Serial.print(" start ms: ");
  Serial.println(start_millis);

  Wire.begin(channel);            // ready
  Wire.onReceive(update_display);  // go.
}

void check_cleanup() {
  if (not running()) return;  // not yet.
  clear();
}

void setup() {
  Serial.begin(9600);
  Serial.println("starting character display.");
  uno_io(hardware);
  Dial dial(hardware);
  Display display(hardware);
  channel = Selector(&dial, &display).get_channel();
  Serial.print("selected: ");
  Serial.println(channel);
  device = new DisplayDevice(hardware);
  device->message = display.message;  // keep showing the channel for a moment
  init_channel(channel);
}

long steps = -1;
void log_status() {
  long prior_steps = steps;
  steps = millis() / channel_display_ms;
  if (prior_steps != steps) {
    Serial.println("c: " + String(channel));
  }
}

void loop() {
  check_cleanup();
  device->refresh();
  log_status();
}
