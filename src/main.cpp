#include "Arduino.h"

#include "main.h"
#include "pins.h"
#include "wifi-sta.h"
#include "pin-io.h"

bool wifiEnabled;

void setup() {
#ifdef USE_SERIAL
  Serial.begin(921600);
  prtl("\n\nStarting...");
#endif

  pinMode(PIN_ENBL_WIFI, INPUT);
  wifiEnabled = !digitalRead(PIN_ENBL_WIFI);

  if(wifiEnabled) wifiSetup();
  pinIoSetup();

  prtl("setup complete\n");
}

void loop() {
  if(wifiEnabled) wifiLoop();
  pinIoLoop();
}