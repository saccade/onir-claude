// lab_version: four digits, lit one at a time. Written by Claude (Anthropic), 2026-10-07.

// pins on the lab's io board
const int segment_pins[7] = {6, 2, 10, 12, 13, 5, 9};  // a b c d e f g
const int point_pin = 11;
const int digit_pins[4] = {7, 4, 3, 8};

char message[5] = "onir";
int wait = 1;  // ms each digit stays lit. try 100.

/*
      aaaaa
    f       b
    f       b
      ggggg
    e       c
    e       c  p
      ddddd
*/
enum { a = 1, b = 2, c = 4, d = 8, e = 16, f = 32, g = 64 };

// what each character looks like; letters in calligraphic order.
int glyph(char ch) {
  if (ch >= 'A' and ch <= 'Z') ch += 'a' - 'A';
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
  return 0;
}

void show(int digit) {
  for (int pin : digit_pins) digitalWrite(pin, HIGH);  // all digits off
  int shape = glyph(message[digit]);
  for (int s = 0; s < 7; s++) digitalWrite(segment_pins[s], shape & (1 << s) ? HIGH : LOW);
  digitalWrite(digit_pins[digit], LOW);  // this one on
}

void setup() {
  for (int pin : segment_pins) pinMode(pin, OUTPUT);
  for (int pin : digit_pins) pinMode(pin, OUTPUT);
  pinMode(point_pin, OUTPUT);
}

void loop() {
  for (int digit = 0; digit < 4; digit++) {
    show(digit);
    delay(wait);
  }
}
