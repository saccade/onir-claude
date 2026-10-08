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
  if (ch == '0') return a | b | c | d | e | f;
  if (ch == '1') return b | c;
  if (ch == '2') return a | b | d | e | g;
  if (ch == '3') return a | b | c | d | g;
  if (ch == '4') return b | c | f | g;
  if (ch == '5') return a | c | d | f | g;
  if (ch == '6') return a | c | d | e | f | g;
  if (ch == '7') return a | b | c;
  if (ch == '8') return a | b | c | d | e | f | g;
  if (ch == '9') return a | b | c | d | f | g;
  if (ch == '-') return g;
  if (ch == '=') return d | g;
  if (ch == '\'') return b;

  if (ch == 'a') return a | b | c | e | f | g;
  if (ch == 'b') return f | e | d | c | g;
  if (ch == 'c') return g | e | d;
  if (ch == 'd') return b | c | d | e | g;
  if (ch == 'e') return a | f | e | d | g;
  if (ch == 'f') return a | f | e | g;
  if (ch == 'g') return g | f | a | b | c | d;
  if (ch == 'h') return f | e | g | c;
  if (ch == 'i') return e;
  if (ch == 'j') return b | c | d | e;
  if (ch == 'k') return f | e | g | b;
  if (ch == 'l') return f | e | d;
  if (ch == 'm') return e | a | c;
  if (ch == 'n') return e | g | c;
  if (ch == 'o') return g | e | d | c;
  if (ch == 'p') return g | b | a | f | e;
  if (ch == 'q') return g | f | a | b | c;
  if (ch == 'r') return e | g;
  if (ch == 's') return f | g | c | d;
  if (ch == 't') return d | e | f | g;
  if (ch == 'u') return b | c | d | e | f;
  if (ch == 'v') return e | d | c;
  if (ch == 'w') return f | d | b;
  if (ch == 'x') return g | b | c;
  if (ch == 'y') return f | g | b | c | d;
  if (ch == 'z') return a | b | g | e;
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
