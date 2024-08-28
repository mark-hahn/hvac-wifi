#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"
#include "pins.h"
#include "pin-io.h"

#define UNKNOWN_LVL 255


const char* pinToName(u8 pin) {
  switch(pin){
    case PIN_IN_Y1 : return (const char *) "Y1" ;
    case PIN_IN_Y1D: return (const char *) "Y1D";
    case PIN_IN_Y2 : return (const char *) "Y2" ;
    case PIN_IN_Y2D: return (const char *) "Y2D";
    case PIN_IN_G  : return (const char *) "G"  ;
    case PIN_IN_W1 : return (const char *) "W1" ;
    case PIN_IN_W2 : return (const char *) "W2" ;
    case PIN_IN_PWR: return (const char *) "PWR";
    default:         return (const char *) "unknown";
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
  char res[128];
  getPinStatus(&res);



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
  pinMode(PIN_G,  INPUT_PULLUP);
  pinMode(PIN_W1, INPUT_PULLUP);
  pinMode(PIN_W2, INPUT_PULLUP);
  pinMode(PIN_Y1, INPUT_PULLUP);
  pinMode(PIN_Y2, INPUT_PULLUP);
}

void hvacLoop() {
  static bool wsWasConnected = false;

  bool wsIsConnected = (wifiConnected && wsConnected());
  if(wsIsConnected != wsWasConnected) {
    prtl("connection changed");
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
