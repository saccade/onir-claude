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
int shape(char ch) {
  if (ch >= 'A' and ch <= 'Z') ch += 'a' - 'A';
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
  return 0;
}

void show(int digit) {
  for (int pin : digit_pins) digitalWrite(pin, HIGH);  // all digits off
  int lit = shape(message[digit]);
  for (int s = 0; s < 7; s++) digitalWrite(segment_pins[s], lit & (1 << s) ? HIGH : LOW);
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
