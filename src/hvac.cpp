#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"
#include "pins.h"

u8  pressPin   = 0;
u8  pressCount = 0;
u32 lastBtnToggle[MAX_PIN] = {0};
u8  lastPinHigh[MAX_PIN]   = {255};

const u8* pinToName(u8 pin){
  switch(pin){
    case PIN_FAN:  return (const u8 *) "fan";
    case PIN_HEAT: return (const u8 *) "heat";
    case PIN_COOL: return (const u8 *) "cool";
    case PIN_SETB: return (const u8 *) "setb";
    case PIN_UP:   return (const u8 *) "up";
    case PIN_DOWN: return (const u8 *) "down";
    case PIN_HOLD: return (const u8 *) "hold";
    case PIN_G:    return (const u8 *) "g";
    case PIN_W1:   return (const u8 *) "w1";
    case PIN_W2:   return (const u8 *) "w2";
    case PIN_Y1:   return (const u8 *) "y1";
    case PIN_Y2:   return (const u8 *) "y2";
    default:       return (const u8 *) "unknown";
  }
}

u8 nameToPin(const u8 *name) {
  if (     strcmp((const char *) name, "fan"    ) == 0) 
        return PIN_FAN;
  else if (strcmp((const char *) name, "heat"   ) == 0) 
        return PIN_HEAT;
  else if (strcmp((const char *) name, "cool"   ) == 0) 
        return PIN_COOL;
  else if (strcmp((const char *) name, "setb") == 0) 
        return PIN_SETB;
  else if (strcmp((const char *) name, "up"     ) == 0) 
        return PIN_UP;
  else if (strcmp((const char *) name, "down"   ) == 0) 
        return PIN_DOWN;
  else if (strcmp((const char *) name, "hold"   ) == 0) 
        return PIN_HOLD;
  else if (strcmp((const char *) name, "g"      ) == 0) 
        return PIN_G;
  else if (strcmp((const char *) name, "w1"     ) == 0) 
        return PIN_W1;
  else if (strcmp((const char *) name, "w2"     ) == 0) 
        return PIN_W2;
  else if (strcmp((const char *) name, "y1"     ) == 0) 
        return PIN_Y1;
  else if (strcmp((const char *) name, "y2"     ) == 0) 
        return PIN_Y2;
  else {
    prtfl("nameToPin unknown pin %s", name);  
    return 0;
  }
}

void hvacRecv(const u8 *data) {
  prtfl("hvacRecv %s", data);
  pressPin = nameToPin(data);
  if (pressPin == 0) return;
  pressCount = 0;
  digitalWrite(pressPin, LOW);
  pinMode(pressPin, OUTPUT);
}


void chkPin(u8 pin) {
  u8 pinHigh = digitalRead(pin);
  if(lastPinHigh[pin] != pinHigh) {
    const u8 *name = pinToName(pin);
    lastPinHigh[pin] = pinHigh;
    prtfl("pin %s changed to %s", 
            name, pinHigh ? "high" : "low");
    char msg[16];
    sprintf(msg, "%s %d", name, pinHigh);
    wsSend((const char *) msg);
  }
}

void chkBtn(u8 pin) {
  u8 pinHigh  = digitalRead(pin);
  u8 lastHigh = lastPinHigh[pin];
  if(lastHigh != pinHigh) {
    lastPinHigh[pin] = pinHigh;
    if(lastHigh == 255) {
      const u8 *name = pinToName(pin);
      prtfl("btn %s pressed", name);
      char msg[32];
      sprintf(msg, "%s pressed", name);
      wsSend((const char *) msg);
      return;
    }
    lastBtnToggle[pin] = millis();
  }
  else if(lastPinHigh[pin] != 255 &&
          millis() - lastBtnToggle[pin] > BTN_TIMEOUT) {
    lastBtnToggle[pin] = 0;
    lastPinHigh[pin]   = 255;
    prtfl("btn %s released", pinToName(pin));
  }
}

void hvacSetup() {
  pinMode(PIN_FAN,     INPUT);
  pinMode(PIN_HEAT,    INPUT);
  pinMode(PIN_COOL,    INPUT);
  pinMode(PIN_SETB, INPUT);
  pinMode(PIN_UP,      INPUT);
  pinMode(PIN_DOWN,    INPUT);
  pinMode(PIN_HOLD,    INPUT);
  pinMode(PIN_G,       INPUT);
  pinMode(PIN_W1,      INPUT);
  pinMode(PIN_W2,      INPUT);
  pinMode(PIN_Y1,      INPUT);
  pinMode(PIN_Y2,      INPUT);
  prtl("hvacSetup complete");
}

void hvacLoop() {
  static u32 lastToggle = 0;
  if (pressPin) {
    if (millis() - lastToggle > 30) {

      pressCount++;
      if (pressCount %2 == 0)
        digitalWrite(pressPin, LOW);
      else
        digitalWrite(pressPin, HIGH);

      if(pressCount > NUM_PRESS_30MS) {
        prtfl("done pressing %s", pinToName(pressPin));
        digitalWrite(pressPin, LOW);
        pinMode(pressPin, INPUT);
        pressPin = 0;
      }

      lastToggle = millis();
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
