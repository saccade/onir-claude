#include "circuits.h"

void uno_io(Hardware hardware) {

  clear(hardware);

  // digit display
  assign(hardware, Fn::dd_a, 6);
  assign(hardware, Fn::dd_b, 2);
  assign(hardware, Fn::dd_c, 10);
  assign(hardware, Fn::dd_d, 12);
  assign(hardware, Fn::dd_e, 13);
  assign(hardware, Fn::dd_f, 5);
  assign(hardware, Fn::dd_g, 9);
  assign(hardware, Fn::dd_p, 11);
  assign(hardware, Fn::dd_1, 7);
  assign(hardware, Fn::dd_2, 4);
  assign(hardware, Fn::dd_3, 3);
  assign(hardware, Fn::dd_4, 8);

  // dial encoder
  assign(hardware, Fn::clock, 17);   // A3
  assign(hardware, Fn::data, 16);    // A2
  assign(hardware, Fn::switch_, 15);  // A1
}

void uno_audio(Hardware hardware) {

  clear(hardware);

  // digit display
  assign(hardware, Fn::dd_a, 6);
  assign(hardware, Fn::dd_b, 2);
  assign(hardware, Fn::dd_c, 17);
  assign(hardware, Fn::dd_d, 15);
  assign(hardware, Fn::dd_e, 14);
  assign(hardware, Fn::dd_f, 5);
  assign(hardware, Fn::dd_g, 18);
  assign(hardware, Fn::dd_p, 16);
  assign(hardware, Fn::dd_1, 7);
  assign(hardware, Fn::dd_2, 4);
  assign(hardware, Fn::dd_3, 3);
  assign(hardware, Fn::dd_4, 19);

  // dial encoder
  assign(hardware, Fn::clock, 8);
  assign(hardware, Fn::data, 9);
  assign(hardware, Fn::switch_, 10);

  Hardware local;  // just the power pins
  clear(local);
  assign(local, Fn::ground, 12);
  power(local);

}

void uno_car(Hardware hardware) {

  clear(hardware);

  assign(hardware, Fn::motor_r_wheel, 10);
  assign(hardware, Fn::motor_l_wheel, 12);

  assign(hardware, Fn::clock, 17);   // A3
  assign(hardware, Fn::data, 16);    // A2
  assign(hardware, Fn::switch_, 15);  // A1

  assign(hardware, Fn::clock_2, 3);
  assign(hardware, Fn::data_2, 4);
  assign(hardware, Fn::switch_2, 5);

  assign(hardware, Fn::vcc, 6);
  assign(hardware, Fn::ground, 7);

  Hardware local;  // just the power pins
  clear(local);
  assign(local, Fn::vcc, 6);
  assign(local, Fn::ground, 7);
  power(local);
}

void uno_driver_test(Hardware hardware) {
  uno_io(hardware);

  // just some pins we don't need for the test demo.
  assign(hardware, Fn::motor_r_wheel, 14);
  assign(hardware, Fn::motor_l_wheel, 15);

}
