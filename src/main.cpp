#include "Arduino.h"

#include "main.h"
#include "pins.h"
#include "wifi-sta.h"

void setup() {
#ifdef USE_SERIAL
  Serial.begin(921600);
  // give monitor time to start
  while (!Serial) sleep(2); 

  prtl("\n\nStarting...");
#endif

  wifiSetup();

  prtfl("\nAll setup complete, millis: %d\n", millis());
}

void loop() {

  wifiLoop();
  
}