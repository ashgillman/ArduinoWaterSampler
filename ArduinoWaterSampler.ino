/*
Arduino Water Sampler

A basic sequencer, waits for an input (from a float sensor indicating presence
of water), then sequences a series of outputs (activating relays to start the
sampler PUMP_PINS).

By Ashley Gillman and Brendan Calvert
*/

#include <Timer.h> // https://github.com/JChristensen/Timer
#define DEBUG 1 // Serial out?

// unit converstion constants
const long HR2MILLI = 60*60*1000;
const long MIN2MILLI = 60*1000;
// pin constants
const int PUMP_PINS[6] = {2, 3, 4, 5, 6, 7};
// time constants
const int DELAYS[6] = {0, 1 * HR2MILLI, 2 * HR2MILLI, 3 * HR2MILLI,
  4 * HR2MILLI, 5 * HR2MILLI};
const int pumpRunTime = 30 * MIN2MILLI;

Timer timer;

int initPumpNo = 0; // Note: zero indexed (0 ~ pump 1)
int currentPumpNo = 0; // Note: zero indexed (0 ~ pump 1)

void setup() {
  if (DEBUG) { Serial.begin(9600); }
  for (int i=0; i<6; i++) {
    timer.after(DELAYS[i],startPump);
    if (DEBUG) {
      Serial.print("initialised pump ");
      Serial.print(i + 1);
      Serial.print(" for ");
      Serial.println(DELAYS[i]);
    }
  }
}

void loop() {
  timer.update();
}

void startPump() {
  digitalWrite(PUMP_PINS[initPumpNo],HIGH); // Pump On
  timer.after(pumpRunTime,stopPump); // Set timer for off
  if (DEBUG) {
    Serial.print("started pump ");
    Serial.println(initPumpNo + 1);
  }
  initPumpNo++;
}

void stopPump() {
  digitalWrite(PUMP_PINS[currentPumpNo],LOW); // Pump Off
  if (DEBUG) {
    Serial.print("stopped pump ");
    Serial.println(currentPumpNo + 1);
  }
  currentPumpNo++;
}
