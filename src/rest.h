#pragma once

// A board left on for days starts over, like a kitchen timer:
// after 160 hours, once nothing has happened for a minute;
// at 24 days no matter what, saying "boot" for five seconds first.
// (millis() runs out of room at 24.8 days.)

void stir();   // something happened: a dial turned, a display lit, a motor moved
bool spent();  // 24 days on: time to say "boot"
void doze();   // call from loop(): starts over when it's time
