// lab_version: one dial, read the plain way. Written by Claude (Anthropic), 2026-10-07.

const int clock = A3;
const int data = A2;
const int button = A1;

int count = 0;
int shown = 0;
int last = 0;    // quarter the shaft sat in last time we looked
int travel = 0;  // quarters turned since it last sat in a detent

// where the shaft sits: 0 in the detent, then 1, 2, 3 turning up.
int quarter() {
  bool c = digitalRead(clock);
  bool d = digitalRead(data);
  if (c and d) return 0;  // in the detent
  if (d) return 1;        // clock let go first
  if (c) return 3;        // data let go first
  return 2;               // both let go
}

// quarters from one spot to the next: up 1, down -1, still or lost 0.
int moved(int from, int to) {
  int step = (to - from + 4) % 4;
  if (step == 1) return 1;
  if (step == 3) return -1;
  return 0;
}

// three ways to count. start at the top.
void every_change(int now) {
  if (now != last) count++;
}

void every_quarter(int now) {
  count += moved(last, now);
}

void every_detent(int now) {
  travel += moved(last, now);
  if (now != 0) return;  // flip only once it falls in
  if (travel > 1) count++;
  if (travel < -1) count--;
  travel = 0;
}

void setup() {
  Serial.begin(9600);
  pinMode(clock, INPUT_PULLUP);
  pinMode(data, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
}

void loop() {
  int now = quarter();
  every_detent(now);
  last = now;

  if (digitalRead(button) == LOW) count = 0;

  if (count != shown) {
    shown = count;
    Serial.println(count);
  }
}
