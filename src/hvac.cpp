#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"
#include "pins.h"

#define UNKNOWN_LVL 255

u8 lastPinLvl [NUM_PINS] = {UNKNOWN_LVL};
u8 pins       [NUM_PINS] = {PIN_G, PIN_W1, PIN_W2, PIN_Y1, PIN_Y2};

const char* pinToName(u8 pin) {
  switch(pin){
    case PIN_G:    return (const char *) "G";
    case PIN_W1:   return (const char *) "W1";
    case PIN_W2:   return (const char *) "W2";
    case PIN_Y1:   return (const char *) "Y1";
    case PIN_Y2:   return (const char *) "Y2";
    default:       return (const char *) "unknown";
  }
}

void sendPinStatus(bool force = false) {
  static char lastRes[64] = "";

  if(!wifiConnected) {
    prtfl("wifi not connected, skipping sendPinStatus");
    return;
  }
  if(!wsConnected()) {
    prtfl("ws not connected, skipping sendPinStatus");
    return;
  }

  char res[64] = "{";
  for (u8 i = 0; i < NUM_PINS; i++) {
    u8 pinLvl = digitalRead(pins[i]);
    lastPinLvl[pins[i]] = pinLvl;
    const char *name = pinToName(pins[i]);
    char msg[16];
    sprintf(msg, "\"%s\":%d,", name, pinLvl);
    strcat (res, msg);
  }
  res[strlen(res) - 1] = '}';

  if(force || strcmp(res, lastRes)) {
    strcpy(lastRes, res);
    wsSend((const char*) res);
    prtfl("ws send: %s", res);
  }
}

// query cmd returns all pin levels
void wsRecv(const u8 *data) {
  prtfl("wsRecv: '%s'", data);
  if (!strcmp((const char*) data, "query")) 
    sendPinStatus(true);
  else 
    prtl("Only query cmd supported, ignoring request");
}

void hvacSetup() {
  pinMode(PIN_G,  INPUT_PULLDOWN);
  pinMode(PIN_W1, INPUT_PULLDOWN);
  pinMode(PIN_W2, INPUT_PULLDOWN);
  pinMode(PIN_Y1, INPUT_PULLDOWN);
  pinMode(PIN_Y2, INPUT_PULLDOWN);
}

void hvacLoop() {
  static bool wsWasConnected = false;

  bool wsIsConnected = (wifiConnected && wsConnected());
  if(wsIsConnected != wsWasConnected) {
    prtl("conn chg");
    wsWasConnected = wsIsConnected;
    if(wsIsConnected) sendPinStatus(true);
    return;
  }
  if(wsIsConnected) {
    for (u8 i = 0; i < NUM_PINS; i++) {
      if(digitalRead(pins[i]) != lastPinLvl[pins[i]])
        sendPinStatus();
    }
  }
}
