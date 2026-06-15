#pragma once

using u_small = unsigned char;
using s_small = signed char;

#define UNSET -1

inline bool const is_unset(const int& value) {
  return value == UNSET;
}

inline void unset(int& value) {
  return value = UNSET;
}

const extern int min_channel;
const extern int max_channel;

// names for logical pin functions.

enum class Function : u_small {
  none,  // default

  // power pair
  ground,
  vcc,

  // dial
  clock,
  data,
  switch_,

  clock_2,
  data_2,
  switch_2,

  // motor names
  motor_main,
  motor_l_wheel,
  motor_r_wheel,
  motor_base,
  motor_shoulder,
  motor_elbow,
  motor_wrist,
  motor_hand,
  motor_end,

  // seven segment digit display outputs

  // segments (anode)
  dd_a,
  dd_b,
  dd_c,
  dd_d,
  dd_e,
  dd_f,
  dd_g,
  dd_p,

  // positions (cathode)
  dd_1,
  dd_2,
  dd_3,
  dd_4,

  // sonar
  trigger,
  echo,

  count,  // last item used for size

};

inline Function& operator++(Function& fn) {
  Function ret = fn;
  fn = (Function)((int)fn + 1);
  return ret;
}

inline bool dys(Function& fn) {
  if (fn == Function::none) return true;
  if (fn == Function::count) return true;
  return false;
}

inline bool is_motor(Function& fn) {
  int i = (int)fn;
  return (i >= (int)Function::motor_main and i <=(int)Function::motor_end);
}

using Hardware = int[(int)Function::count];
extern const Hardware no_hardware;
extern const int hardware_size;

int dispatch(const Hardware& hardware, Function fn);
int dispatch(const Hardware& hardware, Function fn, int mode);

inline bool const is_unset(const Hardware& hardware, Function fn) {
  return is_unset(dispatch(hardware,fn));
}

inline bool const is_set(const Hardware& hardware, Function fn) {
  return not is_unset(dispatch(hardware,fn));
}

int assign(Hardware hardware, Function fn, int pin);
int assign(Hardware hardware, Function fn, int pin, int mode);

bool empty(const Hardware& hardware);
void clear(Hardware hardware);
void power(const Hardware&);

template <typename T>
struct Resource {
  T hardware[(int)Function::count] { };

  T& operator[](Function f) {
    return hardware[(int)f];
  }

  const T& operator[](Function f) const {
    return hardware[(int)f];
  }
};
