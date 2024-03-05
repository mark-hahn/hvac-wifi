#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"
#include "pins.h"

#define UNKNOWN_LVL 255

u8   simuPressPin = 0;
u8   edgeOutCount = 0;
u8   lastPinLvl    [MAX_PIN+1] = {UNKNOWN_LVL};
u32  phyBtnEdgeTime[MAX_PIN+1] = {0};
bool phyPressing   [MAX_PIN+1] = {false};

const u8* pinToName(u8 pin){
  switch(pin){
    case PIN_FAN:  return (const u8 *) "fan";
    case PIN_HEAT: return (const u8 *) "heat";
    case PIN_COOL: return (const u8 *) "cool";
    case PIN_SETB: return (const u8 *) "setb";
    case PIN_UP:   return (const u8 *) "up";
    case PIN_DOWN: return (const u8 *) "down";
    case PIN_HOLD: return (const u8 *) "hold";
    case PIN_G:    return (const u8 *) "G";
    case PIN_W1:   return (const u8 *) "W1";
    case PIN_W2:   return (const u8 *) "W2";
    case PIN_Y1:   return (const u8 *) "Y1";
    case PIN_Y2:   return (const u8 *) "Y2";
    default:       return (const u8 *) "unknown";
  }
}

u8 nameToPin(const u8 *name) {
  if (!strcmp((const char*)    name, "query")) return QUERY_CMD;
  else if(!strcmp((const char*)name, "fan"  )) return PIN_FAN;
  else if(!strcmp((const char*)name, "heat" )) return PIN_HEAT;
  else if(!strcmp((const char*)name, "cool" )) return PIN_COOL;
  else if(!strcmp((const char*)name, "setb" )) return PIN_SETB;
  else if(!strcmp((const char*)name, "up"   )) return PIN_UP;
  else if(!strcmp((const char*)name, "down" )) return PIN_DOWN;
  else if(!strcmp((const char*)name, "hold" )) return PIN_HOLD;
  else if(!strcmp((const char*)name, "G"    )) return PIN_G;
  else if(!strcmp((const char*)name, "W1"   )) return PIN_W1;
  else if(!strcmp((const char*)name, "W2"   )) return PIN_W2;
  else if(!strcmp((const char*)name, "Y1"   )) return PIN_Y1;
  else if(!strcmp((const char*)name, "Y2"   )) return PIN_Y2;
  else {
    prtfl("nameToPin unknown name: '%s'", name);  
    return 0;
  }
}

// cmd is setb, up, down, hold, or query
void wsRecv(const u8 *data) {
  prtfl("wsRecv: '%s'", data);

  // simuPressPin non-zero means simulating button press 
  simuPressPin = nameToPin(data);
  if (!simuPressPin) return;

  if(simuPressPin == QUERY_CMD) {
    // reset lastPinLvl so all pins reported
    for (u8 i = 0; i <= MAX_PIN; i++)
      lastPinLvl[i] = UNKNOWN_LVL;
    simuPressPin = 0;
    return;
  }

  // start phyPressing button setb, up, down, or hold
  prtfl("simulating button down: %4s", pinToName(simuPressPin));
  edgeOutCount = 0;
  digitalWrite(simuPressPin, LOW);
  pinMode(simuPressPin, OUTPUT);
}

void chkPinChg(u8 pin) {
  if(!wifiConnected) {
    lastPinLvl[pin] = UNKNOWN_LVL;
    return;
  }
  u8 pinLvl = digitalRead(pin);
  if(pinLvl != lastPinLvl[pin]) {
    lastPinLvl[pin] = pinLvl;
    const u8 *name = pinToName(pin);
    prtfl("pin %5s: %s", name, pinLvl ? "high" : "low");
    char msg[16];
    sprintf(msg, "%s %d", name, pinLvl);
    wsSend((const char*) msg);
  }
}

void chkBtnPress(u8 pin) {
  if(!wifiConnected) {
    lastPinLvl[pin]  = UNKNOWN_LVL;
    phyBtnEdgeTime[pin] = 0;
    phyPressing[pin]    = false;
    return;
  }
  u8 pinLvl = digitalRead(pin);
  if(lastPinLvl[pin] && !pinLvl) {
    if(lastPinLvl[pin] == UNKNOWN_LVL) {
      lastPinLvl[pin] = pinLvl;
      return;
    }
    // falling edge
    if(!phyPressing[pin]) {
      phyPressing[pin]  = true;
      const u8 *name = pinToName(pin);
      prtfl("physical button down: %4s", name);
      char msg[64];
      sprintf(msg, "%s pressed", name);
      wsSend((const char*) msg);
      phyBtnEdgeTime[pin] = millis();
    }
  }
  else if(phyPressing[pin] &&
          millis() - phyBtnEdgeTime[pin] > BTN_TIME_MS) {
    lastPinLvl[pin]  = UNKNOWN_LVL;
    phyBtnEdgeTime[pin] = 0;
    phyPressing[pin] = false;
    prtfl("physical button up:   %4s", pinToName(pin));
    return;
  }
  lastPinLvl[pin] = pinLvl;
}

void hvacSetup() {
  pinMode(PIN_FAN,  INPUT_PULLUP);
  pinMode(PIN_HEAT, INPUT_PULLUP);
  pinMode(PIN_COOL, INPUT_PULLUP);

  pinMode(PIN_SETB, INPUT_PULLDOWN);
  pinMode(PIN_UP,   INPUT_PULLDOWN);
  pinMode(PIN_DOWN, INPUT_PULLDOWN);
  pinMode(PIN_HOLD, INPUT_PULLDOWN);

  pinMode(PIN_G,    INPUT_PULLUP);
  pinMode(PIN_W1,   INPUT_PULLUP);
  pinMode(PIN_W2,   INPUT_PULLUP);
  pinMode(PIN_Y1,   INPUT_PULLUP);
  pinMode(PIN_Y2,   INPUT_PULLUP);
}

void hvacLoop() {
  static u32 pulseTimeMs = 0;
  if (simuPressPin) {
    // simulating button press
    if (millis() - pulseTimeMs > 
        (edgeOutCount % 2 ? BTN_PRESS_HIGH_MS : BTN_PRESS_LOW_MS)) {
      edgeOutCount++;
      digitalWrite(simuPressPin, edgeOutCount % 2);
      if(edgeOutCount > BTN_EDGE_COUNT) {
        prtfl("simulating button up:   %4s", pinToName(simuPressPin));
        digitalWrite(simuPressPin, LOW);
        pinMode(simuPressPin, INPUT_PULLDOWN);
        simuPressPin = 0;
      }
      pulseTimeMs = millis();
    }
  }
  else {
    // not simulating button press
    // check for level change on pins
    chkPinChg(PIN_FAN);
    chkPinChg(PIN_HEAT);
    chkPinChg(PIN_COOL);
    chkPinChg(PIN_G);
    chkPinChg(PIN_W1);
    chkPinChg(PIN_W2);
    chkPinChg(PIN_Y1);
    chkPinChg(PIN_Y2);

    // check for physical press of physical button
    chkBtnPress(PIN_SETB);
    chkBtnPress(PIN_UP);
    chkBtnPress(PIN_DOWN);
    chkBtnPress(PIN_HOLD);

  }
}
