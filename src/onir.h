#pragma once

// TODO: convert to sketch.

#include "motor/program.h"

class Control;
class Screen;

class Onir {
public:
  Onir(int* channels, int count, const Hardware& hardware = no_hardware);

  void display(char* message);
  void update();

  int now();

  Control* control;
  Screen* screen;

private:
  const Hardware& hardware;
  void act();
  bool go();
  const int step_ms_ = 1000;
  int now_ = UNSET;
};
