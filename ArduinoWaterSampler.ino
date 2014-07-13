/*
Arduino Water Sampler

A basic sequencer, waits for an input (from a float sensor indicating presence
of water), then sequences a series of outputs (activating relays to start the
sampler PUMP_PINS).

Properties
0: Pump 1 mins
1: Pump 2 mins
2: Pump 3 mins
3: Pump 4 mins
4: Pump 5 mins
5: Pump 6 mins
7: Pump runtime

By Ashley Gillman and Brendan Calvert
*/
#include <avr/wdt.h>
#include <Timer.h> // https://github.com/JChristensen/Timer
#include <SPI.h>
#include <Adafruit_GFX.h> // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_PCD8544.h>
// https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library
#include <ButtonEvent.h>
#include <Properties.h> // https://github.com/usefulthink/ebl-arduino

#define DEBUG 1 // Serial out?

// unit converstion constants
const long HR2MILLI = 60*60*1000L;
const long MIN2MILLI = 60*1000L;
const long SEC2MILLI = 1000L;

// pin constants
const int RST = 3;
const int CE = 4;
const int DC = 5;
const int DIN = 6;
const int CLK = 7;
const int PUMP_PINS[6] = {8, 9, 10, 11, 12, 13};
const int btnPin = 0; // note: A0

// analog button values
const int btnDev = 100;
const int holdTime = 500;
const int doubleTime = 200;
const int upBtn = 1023 / 3;
const int downBtn = 1023 / 2;
const int nextBtn = 1023;

Timer timer;
Adafruit_PCD8544 display = Adafruit_PCD8544(CLK, DIN, DC, CE, RST);

boolean error = false;
boolean reqRestart = false;
int cursorPos = 0;
boolean blinker = false;
int initPumpNo = 0; // Note: zero indexed (0 ~ pump 1)
int currentPumpNo = 0; // Note: zero indexed (0 ~ pump 1)
boolean pumpActive[6] = {0, 0, 0, 0, 0, 0};

void setup() {
  // init
  if (DEBUG) { Serial.begin(9600); }
  display.begin();
  display.setContrast(50);
  
  // load from EEPROM
  if (!Properties.load()) { error = true; }
  
  // buttons
  ButtonEvent.initialCapacity = sizeof(ButtonInformation)*3;
  ButtonEvent.addButton(btnPin, nextBtn, btnDev, incrementCursor, NULL, NULL,
    holdTime, decrementCursor, doubleTime);
  ButtonEvent.addButton(btnPin, upBtn, btnDev, changeVal, NULL,
    bigChangeVal, holdTime, NULL, doubleTime);
  ButtonEvent.addButton(btnPin, downBtn, btnDev, changeVal, NULL,
    bigChangeVal, holdTime, NULL, doubleTime);
  
  // setup pumps
  for (int i=0; i<6; i++) {
    pinMode(PUMP_PINS[i],OUTPUT);
    timer.after(Properties.getInt(i),startPump);
    if (DEBUG) {
      Serial.print("initialised pump ");
      Serial.print(i + 1);
      Serial.print(" for ");
      Serial.println(Properties.getInt(i));
    }
  }
  
  //blinker
  timer.every(500, blink);
}

void loop() {
  timer.update();
  ButtonEvent.loop();
  if (cursorPos < 12) { displayTimes(); } // page 1
  else { displayRuntime(); } // page 2
}

void startPump() {
  digitalWrite(PUMP_PINS[initPumpNo],HIGH); // Pump On
  pumpActive[initPumpNo] = true;
  timer.after(Properties.getInt(6),stopPump); // Set timer for runtime
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

void modify(int pump, int mins) {
  reqRestart = true;
  Properties.set(pump, max(Properties.getInt(pump) + mins, 0)); // prevent neg
  Properties.save();
}

void saveAndRestart() {
  displayBig("Saving");
  displayBig("Config");
  delay(800);
  Properties.save();
  wdt_enable(WDTO_15MS);
  while(true) {}
}

void displayTimes() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  for (int i=0; i<6; i++) {
    // P<x>: <y>h <z>min
    if (pumpActive[i]) { display.setTextColor(WHITE, BLACK); }
    else { display.setTextColor(BLACK); } // highlight active pumps
    display.print("P");
    display.print(i + 1);
    display.setTextColor(BLACK);
    display.print(": ");
    if ((cursorPos == 2*i) & blinker) {
      display.setTextColor(WHITE, BLACK); // blink
    }
    display.print(Properties.getInt(i) / 60);
    display.setTextColor(BLACK);
    display.print("h ");
    if ((cursorPos == 2*i+1) & blinker) {
      display.setTextColor(WHITE, BLACK); // blink
    }
    display.print(Properties.getInt(i) % 60);
    display.setTextColor(BLACK);
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
    if ((cursorPos == 12) & blinker) {
      display.setTextColor(WHITE, BLACK); // blink
    }
  display.print(Properties.getInt(6) / 60);
  display.setTextColor(BLACK);
  display.print("h ");
    if ((cursorPos == 13) & blinker) {
      display.setTextColor(WHITE, BLACK); // blink
    }
  display.print(Properties.getInt(6) % 60);
  display.setTextColor(BLACK);
  display.println("min");
  display.display();
}

void displayBig(String str) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.println(str);
  display.display();
}

void incrementCursor(ButtonInformation* Sender) {
  cursorPos = (cursorPos + 1) % 14;
  if ((cursorPos == 0) & reqRestart) { saveAndRestart(); }
}

void decrementCursor(ButtonInformation* Sender) {
  cursorPos = max(cursorPos - 2, 0);
}

void changeVal(ButtonInformation* Sender) {
  int inc;
  if (cursorPos % 2 == 0) {
    inc = 60; //mins
  } else {
    inc = 1; // min
  }
  if (Sender->analogValue == upBtn) {
    modify(cursorPos/2, inc);
    if (DEBUG) {
      Serial.print("increment Pump ");
      Serial.println(cursorPos/2+1);
    }
  } else if (Sender->analogValue == downBtn) {
    modify(cursorPos/2, -inc);
    if (DEBUG) {
      Serial.print("decrement Pump ");
      Serial.println(cursorPos/2+1);
    }
  }
}

void bigChangeVal(ButtonInformation* Sender) {
  int inc;
  if (cursorPos % 2 == 0) {
    inc = 540; //mins
  } else {
    inc = 9; // min
  }
  if (Sender->analogValue == upBtn) {
    modify(cursorPos/2, inc);
  } else if (Sender->analogValue == downBtn) {
    modify(cursorPos/2, -inc);
  }
}


void blink() {
  blinker = !blinker;
}
