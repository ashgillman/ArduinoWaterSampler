/*
Arduino Water Sampler

A basic sequencer, waits for an input (from a float sensor indicating presence
of water), then sequences a series of outputs (activating relays to start the
sampler pumps).

By Ashley Gillman and the Amazing Brendan Calvert
*/

//#include "/libraries/Timer/Timer.h" // https://github.com/JChristensen/Timer

// unit converstion constants
const long hr2milli = 60*60*1000;
const long min2milli = 60*1000;

// pin constants
const int pumps[6] = {2, 3, 4, 5, 6, 7};

// time constants
const int delays[6] = {0, 1 * hr2milli, 2 * hr2milli, 3 * hr2milli,
  4 * hr2milli, 5 * hr2milli};
const int pumpRunTime = 30 * min2milli;

Timer timer;

void setup() {
  Serial.begin(9600);
  for (int i=0; i<6; i++) {
    timer.after(delays[i],startPump(i));
  }
}

void loop() {
  timer.update();
}

void startPump(int pumpNo) {
  digitalWrite(pumps[pumpNo],HIGH);
  timer.after(pumpRunTime,stopPump(pumpNo));
}

void stopPump(int pumpNo) {
  digitalWrite(pumps[pumpNo],LOW);
}
