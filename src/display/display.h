#pragma once

#include "data.h"
#include "timing.h"
#include "util.h"

// abstract interface for a seven-segment display

class DisplayDevice;

// Client for seven-segment display.
class Display {
public:
  Display(const Hardware& hardware = no_hardware);

  Display(int ch);

  static int call(Change& change);
  int call();

  void init(int channel);

  void attach(DisplayDevice* d) {
    device = d;
  }

  void set_value(int value) {
    set_digits(value);
  }

  void set_point(int place) {
    message.point = place;
  }

  void clear_point() {
    message.point = UNSET;
  }

  void put(char ch, int place) {
    message.chars[place] = ch;
  }

  void put_str(const char* str) {  // beyond 4 dropped
    for (int i = 0; i < 4; i++) {
      put(str[i], i);
    }
  }

  void clear() {
    for (int i = 0; i < 4; i++) {
      put(' ', i);
    }
    clear_point();
  }

  // call in loop()
  void refresh();

  void show_zero(bool show) {
    shows_zero = show;
  }

  Message message;

private:
  DisplayDevice* device = 0;
  bool shows_zero = false;

  Rhythm rhythm;
  Change change;

  void set_digits(int nnn) {
    for (int i = 0; i < 4; i++) {
      put(0, i);  // 0 gives a blank display
    }
    if ((nnn == 0) && !shows_zero) return;
    int sign = nnn ? nnn / absv(nnn) : 0;
    if (sign < 0) {
      put('-', 0);
    }
    int value = absv(nnn);
    // assign value's lowest three place values to the display's digits.
    // higher place values are discarded.
    for (int k = 3; k >= 1; k--) {
      int modulus = 1;  // C++, y u no (int) pow?
      for (int j = 0; j < 4 - k - 1; j++) {
        modulus *= 10;
      }
      put('0' + (value / modulus) % 10, k);
      if (!(value / (modulus * 10))) return;
    }
  }

};
