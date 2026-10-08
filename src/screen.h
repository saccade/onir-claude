#pragma once

#include "display/display.h"
#include "control.h"
#include "log.h"

const static int margin = 128;

// Splits a string across a handful of displays and keeps them refreshed.
class Screen {

public:
  Screen(const Hardware& hardware) : control_(new Control(hardware)) {
    clear();
  }

  Screen(Control* control) : control_(control) {
    clear();
  }

  void clear() {
    erase();
    if (fan_start_) {
      fan();
    }
    for (Unit* unit : control_->units) {
      if (unit) {
        unit->dial.zero();
      }
    }
  }

  void fan_start(bool b) {
    fan_start_ = b;
  }

  void erase() {
    for (int i = 0; i < margin; i++) {
      display_[i] = 0;
    }
    for (int i = 0; i < BANDS; i++) {
      baselines_[i] = 0;
    }
    message_ = 0;
    index_ = 0;
  }

  void display(const char* message) {
    message_ = message;
    record();
    show();
  }

  void pan(int move) {
    index_ += move;
    show();
  }

  void seek(int index) {
    index_ = index;
    show();
  }

  int index() const { return index_; }
  int width() const { return length(display_); }

  void show() {
    Unit* const local = control_->local();
    for (int channel = 0; channel < BANDS; channel++) {
      if (Unit* unit = control_->units[channel]) {
        for (int spot = 0; spot < 4; spot++) {
          int offset = spot;
          offset += index_;                             // unit pan
          offset += baselines_[channel];                // channel baseline
          // late read -> fewer dial side-effects
          if (local) {
            offset += local->dial.value();              // control offset
          }
          if (unit != local) {
            offset += unit->dial.value();               // device offset
          }
          unit->display.put(char_at(offset), spot);
        }
      }
    }
  }

private:
  char char_at(int index) {
    return display_[mod(width(), index)];;
  }

  void fan() {  // Sequence channel offsets from zero.
    index_ = 0;
    for (int channel = 0; channel < BANDS; channel++) {
      baselines_[channel] = 0;
    }
    int count = 0;
    for (int channel = 0; channel < BANDS; channel++) {
      if (control_->units[channel]) {
        baselines_[channel] = 4 * count++;
      }
    }
    show();
  }

  int record() {                                // may pad start/end:
    int receipt = length(message_);             // find the 0
    int length = 0;
    if (receipt and message_[0] != ' ') {
      display_[length++] = ' ';                 // left-pad
    }
    for (int i = 0; (i < receipt) and (length < margin - 1);) {
      display_[length++] = message_[i++];       // copy message
    }
    while (length < 4) {                        // pad to 4
      display_[length++] = ' ';
    }
    if (display_[length-1] != ' '  and length < margin - 2) {
      display_[length++] = ' ';                 // right-pad
    }
    display_[length] = 0;                       // terminate
    return length;
  }

  static int mod(int modulus, int value) {   // oh, C++...
    int ret = value % modulus;
    if (ret < 0) { ret += modulus; }
    return ret;
  }

  static int length(const char* str) {
    int length = 0;
    while (str and str[length]) { length++; }
    return length;
  }

  Control* const control_;
  const char* message_;
  char display_[margin];
  int index_;
  int baselines_[BANDS];
  bool fan_start_ = true;
};
