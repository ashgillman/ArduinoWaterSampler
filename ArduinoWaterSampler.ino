/*
Arduino Water Sampler

A basic sequencer, waits for an input (from a float sensor indicating presence
of water), then sequences a series of outputs (activating relays to start the
sampler PUMP_PINS).

By Ashley Gillman and Brendan Calvert
*/

#include <Timer.h> // https://github.com/JChristensen/Timer
#include <SPI.h>
#include <Adafruit_GFX.h> // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_PCD8544.h>
// https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library
#include <ButtonEvent.h> // https://github.com/usefulthink/ebl-arduino

#define DEBUG 1 // Serial out?

// unit converstion constants
const long HR2MILLI = 60*60*1000L;
const long MIN2MILLI = 60*1000L;
const long SEC2MILLI = 1000L;

// pin constants
const int PUMP_PINS[6] = {8, 9, 10, 11, 12, 13};

// time constants
const long DELAYS[6] = {
  0,
  1 * MIN2MILLI,
  2 * MIN2MILLI,
  3 * MIN2MILLI,
  4 * MIN2MILLI,
  5 * MIN2MILLI
};
const long pumpRunTime = 60 * SEC2MILLI;

// analog button values
const int btnPin = 0;
const int btnDev = 100;
const int holdTime = 500;
const int doubleTime = 0;
const int upBtn = 1023 / 3;
const int downBtn = 1023 / 2;
const int nextBtn = 1023;

Timer timer;
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

int initPumpNo = 0; // Note: zero indexed (0 ~ pump 1)
int currentPumpNo = 0; // Note: zero indexed (0 ~ pump 1)
boolean pumpActive[6] = {0, 0, 0, 0, 0, 0};
boolean displayScreen = true;

void setup() {
  // init
  if (DEBUG) { Serial.begin(9600); }
  display.begin();
  display.setContrast(50);
  
  // buttons
  ButtonEvent.initialCapacity = sizeof(ButtonInformation)*3;
  ButtonEvent.addButton(btnPin, nextBtn, btnDev, toggleDisplay, NULL, NULL,
    holdTime, NULL, doubleTime);
  
  // setup pumps
  for (int i=0; i<6; i++) {
    pinMode(PUMP_PINS[i],OUTPUT);
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
  ButtonEvent.loop();
  if (displayScreen) { displayTimes(); }
  else { displayRuntime(); }
}

void startPump() {
  digitalWrite(PUMP_PINS[initPumpNo],HIGH); // Pump On
  pumpActive[initPumpNo] = true;
  timer.after(pumpRunTime,stopPump); // Set timer for off
  if (DEBUG) {
    Serial.print("started pump ");
    Serial.print(initPumpNo + 1);
    Serial.print(" on pin ");
    Serial.println(PUMP_PINS[initPumpNo]);
  }
  initPumpNo++;
}

void stopPump() {
  digitalWrite(PUMP_PINS[currentPumpNo],LOW); // Pump Off
  pumpActive[currentPumpNo] = false;
  if (DEBUG) {
    Serial.print("stopped pump ");
    Serial.print(currentPumpNo + 1);
    Serial.print(" on pin ");
    Serial.println(PUMP_PINS[currentPumpNo]);
  }
  currentPumpNo++;
}

void displayTimes() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  for (int i=0; i<6; i++) {
    if (pumpActive[i]) { display.setTextColor(WHITE, BLACK); }
    else { display.setTextColor(BLACK); }
    display.print("P");
    display.print(i + 1);
    display.setTextColor(BLACK);
    display.print(": ");
    display.print(DELAYS[i]/HR2MILLI);
    display.print("h ");
    display.print(DELAYS[i]/MIN2MILLI);
    display.println("min");
  }
  display.display();
}

void displayRuntime() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.println("Runtime:");
  display.print(pumpRunTime/HR2MILLI);
  display.print("h ");
  display.print(pumpRunTime/MIN2MILLI);
  display.println("min");
  display.display();
}

void toggleDisplay(ButtonInformation* Sender) {
  displayScreen = !displayScreen;
  Serial.println(displayScreen);
}
