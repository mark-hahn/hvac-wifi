#include "Arduino.h"

#include "main.h"
#include "wifi-sta.h"
#include "hvac.h"
#include "pin-io.h"

void setup() {
#ifdef USE_SERIAL
  Serial.begin(921600);
  prtl("\n\nStarting...");
#endif

  wifiSetup();
  hvacSetup();
  pinIoSetup();

  prtl("setup complete\n");
}

void loop() {
  // debug measure loop delay
  static u32 MainLoopCnt = 0;
  static u32 lastMillis  = 0;
  // delay must be less than 2ms
  if((++MainLoopCnt % 10000) == 0) {
    prtfl("sample loop delay ms: %n", (millis() - lastMillis));
  }
  lastMillis = millis();

  wifiLoop();
  hvacLoop();
  pinIoLoop();
}