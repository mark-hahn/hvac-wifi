#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"
#include "pins.h"
#include "pin-io.h"

#define UNKNOWN_LVL 255

// query cmd returns all pin levels
void wsRecv(const u8 *data) {
  prtfl("wsRecv: '%s'", data);
  if (!strcmp((const char*) data, "query")) 
    sendChangedPins(true);
  else 
    prtl("Only query cmd supported, ignoring request");
}

void hvacSetup() {
}

void hvacLoop() {
  static bool wsWasConnected = false;

  bool wsIsConnected = wsConnected();
  if(wsIsConnected != wsWasConnected) {
    prtfl("wsIsConnected changed to %s",
           wsIsConnected ? "true" : "false");
    wsWasConnected = wsIsConnected;
    if(wsIsConnected) sendChangedPins(true);
  }
}
