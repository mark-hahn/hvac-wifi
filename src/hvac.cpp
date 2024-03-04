#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"
#include "pins.h"

#define UNKNOWN_LVL 255

u8   pressPin  = 0;
u8   edgeCount = 0;
u8   lastPinLvl [MAX_PIN+1] = {UNKNOWN_LVL};
u32  btnEdgeTime[MAX_PIN+1] = {0};
bool pressing   [MAX_PIN+1] = {false};

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
  // prtfl("nameToPin %s", name);
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
  pressPin = nameToPin(data);
  if (!pressPin) return;

  if(pressPin == QUERY_CMD) {
    // reset lastPinLvl so all pins reported
    for (u8 i = 0; i <= MAX_PIN; i++)
      lastPinLvl[i] = UNKNOWN_LVL;
    pressPin = 0;
    return;
  }

  // start pressing button setb, up, down, or hold
  prtfl("press cmd: %s", data);
  edgeCount = 0;
  digitalWrite(pressPin, LOW);
  pinMode(pressPin, OUTPUT);
}

void chkPin(u8 pin) {
  if(!wifiConnected) {
    lastPinLvl[pin] = UNKNOWN_LVL;
    return;
  }
  u8 pinLvl = digitalRead(pin);
  if(pinLvl != lastPinLvl[pin]) {
    lastPinLvl[pin] = pinLvl;
    const u8 *name = pinToName(pin);
    prtfl("pin %5s is %s", name, pinLvl ? "high" : "low");
    char msg[16];
    sprintf(msg, "%s %d", name, pinLvl);
    wsSend((const char*) msg);
  }
}

void chkBtn(u8 pin) {
  if(!wifiConnected) {
    lastPinLvl[pin]  = UNKNOWN_LVL;
    btnEdgeTime[pin] = 0;
    pressing[pin]    = false;
    return;
  }
  u8 pinLvl = digitalRead(pin);
  if(lastPinLvl[pin] && !pinLvl) {
    if(lastPinLvl[pin] == UNKNOWN_LVL) {
      lastPinLvl[pin] = pinLvl;
      return;
    }
    // falling edge
    if(!pressing[pin]) {
      pressing[pin]    = true;
      const u8 *name = pinToName(pin);
      prtfl("pressing %5s", name);
      char msg[64];
      sprintf(msg, "%s pressed", name);
      wsSend((const char*) msg);
      btnEdgeTime[pin] = millis();
    }
  }
  else if(pressing[pin] &&
          millis() - btnEdgeTime[pin] > BTN_TIME_MS) {
    lastPinLvl[pin]  = UNKNOWN_LVL;
    btnEdgeTime[pin] = 0;
    pressing[pin] = false;
    prtfl("releasing %5s", pinToName(pin));
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

  pinMode(PIN_G,    INPUT);
  pinMode(PIN_W1,   INPUT);
  pinMode(PIN_W2,   INPUT);
  pinMode(PIN_Y1,   INPUT);
  pinMode(PIN_Y2,   INPUT);
  prtl("hvacSetup complete");
}

void hvacLoop() {
  static u32 pulseTimeMs = 0;
  if (pressPin) {
    if (millis() - pulseTimeMs > 
        (edgeCount % 2 ? BTN_PRESS_HIGH_MS : BTN_PRESS_LOW_MS)) {
      edgeCount++;
      prtl(edgeCount % 2 ? "pressing rising edge" : "pressing falling edge");
      digitalWrite(pressPin, edgeCount % 2);
      if(edgeCount > BTN_EDGE_COUNT) {
        prtl("done pressing");
        prtl("done pressing");
        prtl("done pressing");
        prtfl("done pressing %s", pinToName(pressPin));
        digitalWrite(pressPin, LOW);
        pinMode(pressPin, INPUT_PULLDOWN);
        pressPin = 0;
      }
      pulseTimeMs = millis();
    }
  }
  // don't check pins when pressing a button
  else {
    chkPin(PIN_FAN);
    chkPin(PIN_HEAT);
    chkPin(PIN_COOL);

    chkBtn(PIN_SETB);
    chkBtn(PIN_UP);
    chkBtn(PIN_DOWN);
    chkBtn(PIN_HOLD);

    chkPin(PIN_G);
    chkPin(PIN_W1);
    chkPin(PIN_W2);
    chkPin(PIN_Y1);
    chkPin(PIN_Y2);
  }
}
