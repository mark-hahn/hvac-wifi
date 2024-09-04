#include "Arduino.h"

#include "main.h"
#include "pins.h"
#include "wifi-sta.h"
#include "pin-io.h"

bool wifiEnabled;

void setup() {
#ifdef USE_SERIAL
  Serial.begin(921600);
  while (!Serial);
  prtl("\n\nStarting...");
#endif

  pinMode(PIN_ENBL_WIFI, INPUT);
  wifiEnabled = !digitalRead(PIN_ENBL_WIFI);

  if(wifiEnabled) wifiSetup();
  pinIoSetup();

  prtl("setup complete\n");
}

// debug
u32 pwrPulses   = 0;
u32 inPwrPulses = 0;

void loop() {
  // measure loop delay for 10 secs
  // delay must be less than 4ms
  static u32  firstMillis    = 0;
  static u32  lastMillis     = 0;
  static u32  worstLoopDelay = 0;
  static u32  worstDelayTime = 0;
  static bool testing        = true;
  if (testing) {
    u32 now = millis();
    if (firstMillis == 0) firstMillis = now;
    if (lastMillis == 0)  lastMillis = now;
    u32 delay = now - lastMillis;
    lastMillis = now;
    if(delay > worstLoopDelay) {
      worstLoopDelay = delay;
      worstDelayTime = now;
    }
    if((now-firstMillis) > 10000) {
      prtl("testing complete");
      prtl(worstLoopDelay);
      prtl(worstDelayTime);
      prtl(pwrPulses);
      prtl(inPwrPulses);
    
      // prtfl("worst loop delay ms: %n, time: %n", 
      //        worstLoopDelay, worstDelayTime);
      // prtfl("power pulses: %n, inside: %n", 
      //        pwrPulses, inPwrPulses);
      testing = false;
    }
  }

  if(wifiEnabled) wifiLoop();
  pinIoLoop();
}