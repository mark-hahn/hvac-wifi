#include "Arduino.h"

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"

void setup() {
#ifdef USE_SERIAL
  Serial.begin(921600);
  prtl("\n\nStarting...");
#endif

  wifiSetup();
  hvacSetup();
  prtl("setup complete\n");
}

void loop() {
  wifiLoop();
  hvacLoop();
}