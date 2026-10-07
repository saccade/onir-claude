#include "display_device.h"

#include "Arduino.h"
#include "rest.h"
#include "Wire.h"

DisplayDevice::DisplayDevice(const Hardware& hardware) : hardware(hardware) {
  for (int i = 0; i < 7; i++) {
    segments[i] = (Function)(i + (int)Fn::dd_a);
  }
  for (int i = 0; i < 4; i++) {
    positions[i] = (Function)(i + (int)Fn::dd_1);
  }
  for (int i = (int)Function::dd_a; i <= (int)Function::dd_4; i++) {
    pinMode(dispatch(hardware, (Function)i), OUTPUT);
  }
  message.clear();
  blank();
}

// segments by name, as drawn in display_device.h.
namespace {
enum : u_small { a = 1, b = 2, c = 4, d = 8, e = 16, f = 32, g = 64 };
}

// what each character looks like; letters in calligraphic order.
static u_small glyph(char ch) {
  if (ch >= 'A' and ch <= 'Z') ch += 'a' - 'A';  // people will use capitals; do what they mean.
  switch (ch) {
    case '0': return a | b | c | d | e | f;
    case '1': return b | c;
    case '2': return a | b | d | e | g;
    case '3': return a | b | c | d | g;
    case '4': return b | c | f | g;
    case '5': return a | c | d | f | g;
    case '6': return a | c | d | e | f | g;
    case '7': return a | b | c;
    case '8': return a | b | c | d | e | f | g;
    case '9': return a | b | c | d | f | g;
    case '-': return g;
    case '=': return d | g;
    case '\'': return b;

    case 'a': return a | b | c | e | f | g;
    case 'b': return f | e | d | c | g;
    case 'c': return g | e | d;
    case 'd': return b | c | d | e | g;
    case 'e': return a | f | e | d | g;
    case 'f': return a | f | e | g;
    case 'g': return g | f | a | b | c | d;
    case 'h': return f | e | g | c;
    case 'i': return e;
    case 'j': return b | c | d | e;
    case 'k': return f | e | g | b;
    case 'l': return f | e | d;
    case 'm': return e | a | c;
    case 'n': return e | g | c;
    case 'o': return g | e | d | c;
    case 'p': return g | b | a | f | e;
    case 'q': return g | f | a | b | c;
    case 'r': return e | g;
    case 's': return f | g | c | d;
    case 't': return d | e | f | g;
    case 'u': return b | c | d | e | f;
    case 'v': return e | d | c;
    case 'w': return f | d | b;
    case 'x': return g | b | c;
    case 'y': return f | g | b | c | d;
    case 'z': return a | b | g | e;
  }
  return 0;  // anything else is blank.
}

bool showing(const Message& message) {
  for (int i = 0; i < 4; i++) {
    if (glyph(message.chars[i])) return true;
  }
  return message.point >= 0 and message.point < 4;
}

void DisplayDevice::blank() {
  for (Function segment : segments) {
    pin_low(segment);
  }
  set_point_pin(LOW);
  for (Function position : positions) {
    pin_high(position);
  }
}

void DisplayDevice::set_fn_pin(Function fn, bool val) {
  digitalWrite(dispatch(hardware, fn), val);
}

void DisplayDevice::pin_high(Function fn) {
  set_fn_pin(fn, HIGH);
}

void DisplayDevice::pin_low(Function fn) {
  set_fn_pin(fn, LOW);
}

void DisplayDevice::set_point_pin(bool val) {
  set_fn_pin(Fn::dd_p, val);
}

// light the next digit, once the last has had its millisecond.
// taking turns, no digit is skipped however slow the loop.
void DisplayDevice::refresh() {
  unsigned long now = millis();
  if (position != UNSET and now - shown_at < ms_per_digit) return;
  shown_at = now;
  position = (position + 1) % 4;

  if (spent()) {  // say goodbye
    message.chars[0] = 'b';
    message.chars[1] = 'o';
    message.chars[2] = 'o';
    message.chars[3] = 't';
    message.point = UNSET;
  }
  if (showing(message)) stir();
  doze();

  blank();
  u_small shape = glyph(message.chars[position]);
  for (int i = 0; i < 7; i++) {
    if (shape & (1 << i)) {
      pin_high(segments[i]);
    }
  }
  if (message.point == position) {
    set_point_pin(HIGH);
  }
  pin_low(positions[position]);
}
