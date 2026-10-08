#pragma once

#include "data.h"
#include "dial/dial.h"

struct Link {
  Dial* dial { };
  Instruction instruction { };

  operator bool() const {
    return dial;
  }

  Link() { };
  Link(const Hardware&);
  Link(Dial*);
  Link(Link*);
};

class Panel {
public:
  Panel() { }
  Panel(const Hardware&);

  Link*& operator[](Function fn) {
    return dials[(int)fn];
  }
protected:
  Link* dials[(int)Function::count] = { };
};

class Stem : public Panel {  // a pair of linked dials (ideally mounted co-axially)
public:

  Link*& left();
  Link*& right();

  operator bool() const {
    return left()->dial and right()->dial;
  }

  Stem() { };
  Stem(Dial* left, Dial* right);
  Stem(const Hardware& hardware);

private:
  Function fn_left =  Function::motor_l_wheel;
  Function fn_right = Function::motor_r_wheel;
};
