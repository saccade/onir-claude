#pragma once

#include "data.h"
#include "util.h"

void log(const Instruction& s);

void print_motion(const Motion& motion, bool newline = true);
void print_reading(const Reading& reading, bool newline = true);

void print_instruction(const Instruction& s);
void print_todo(const Instruction& s);

void print_display(const Message& display);

class Unit;
void log_io(Unit* unit);

extern int log_winks;
extern bool explain;  // say each step out loud (slow); false: run smooth and silent
int memcheck();
