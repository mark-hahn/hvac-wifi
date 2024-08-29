#include <Arduino.h>
#include <WiFi.h>

#include "pin-io.h"
#include "main.h"
#include "pins.h"
#include "wifi-sta.h"

#define DEBOUNCE_DELAY_MS 2
#define DEFAULT_YDELAY_MS 5000

u8 ledInPins[] = {
  PIN_IN_Y1 ,   
  PIN_IN_Y1D,   
  PIN_IN_Y2 ,   
  PIN_IN_Y2D,   
  PIN_IN_G  ,   
  PIN_IN_W1 ,   
  PIN_IN_W2 ,   
  PIN_IN_PWR
};

#define FAN_PIN_IDX  4

u8 ledOutPins[sizeof ledInPins] = {
  PIN_LED_Y1  ,
  PIN_LED_Y1D ,
  PIN_LED_Y2  ,
  PIN_LED_Y2D ,
  PIN_LED_G   ,
  PIN_LED_W1  ,
  PIN_LED_W2  ,
  PIN_LED_PWR
};

const char* pinNames[] = {
  (const char *) "Y1",
  (const char *) "Y1D",
  (const char *) "Y2" ,
  (const char *) "Y2D",
  (const char *) "G"  ,
  (const char *) "W1" ,
  (const char *) "W2" ,
  (const char *) "PWR"
};

int yDelayMs = DEFAULT_YDELAY_MS;
void setYDelay(int delay) {
  yDelayMs = delay;
}

u8   inPinLvls[sizeof ledInPins]   = {1};
u8   outPinLvls[sizeof ledOutPins] = {0};
bool pinChanged[sizeof ledInPins]  = {false};

// send pin status to server when any pin changes
void sendPinVals(bool forceAll) {
  char json[128];
  json[0] = '{';
  json[1] = 0;
  for (int pinIdx = 0; pinIdx < (sizeof ledOutPins); pinIdx++) {
    if(forceAll || pinChanged[pinIdx]) {
      const char* name = pinNames[pinIdx];
      u8 pinLvl        = outPinLvls[pinIdx];
      char msg[32];
      if (pinLvl) sprintf(msg, "\"%s\":true,",  name);
      else        sprintf(msg, "\"%s\":false,", name);
      strcat(json, msg);
    }
  }
  json[strlen(json)-1] = '}';
  wsSend(json);
}

u32 lastPwrFallMs = 0;

void IRAM_ATTR handlePowerPinFall() {
  detachInterrupt(PIN_IN_PWR);
  lastPwrFallMs = millis();
}

void pinIoSetup() {
  for(int pinIdx = 0; pinIdx < (sizeof ledInPins);  pinIdx++)
    pinMode(ledInPins[pinIdx],  INPUT);
  for(int pinIdx = 0; pinIdx < (sizeof ledOutPins); pinIdx++)
    pinMode(ledOutPins[pinIdx], OUTPUT);
  pinMode(PIN_ENBL_WIFI,   INPUT);
  pinMode(PIN_OPEN_Y1,     OUTPUT);  
  pinMode(PIN_OPEN_Y2,     OUTPUT);  
  digitalWrite(PIN_OPEN_Y1, LOW);
  digitalWrite(PIN_OPEN_Y2, LOW);
  attachInterrupt(
    PIN_IN_PWR, handlePowerPinFall, FALLING);  
}

void pinIoLoop() {
  static u32  fanOnTime        = 0;
  static bool waitingForYDelay = false;
  static bool wsWasConnected   = false;

  u32 now = millis();
  if(lastPwrFallMs &&
      (now - lastPwrFallMs) > DEBOUNCE_DELAY_MS) {
    lastPwrFallMs = 0;

    // power fall debounced, arm interrupt for next fall
    attachInterrupt(
      PIN_IN_PWR, handlePowerPinFall, FALLING); 

    // inside power pulse, all pin inputs valid
    bool havePinChg    = false;
    bool haveFanPinChg = false;
    for(int pinIdx = 0; 
        pinIdx < (sizeof ledInPins);  pinIdx++) {
      int inPinGpioNum  = ledInPins[pinIdx];
      u8  inPinLvl      = digitalRead(inPinGpioNum);
      inPinLvls[pinIdx] = inPinLvl;
      if (inPinLvl == outPinLvls[pinIdx]) {
        // input pin changed
        pinChanged[pinIdx] = true;
        havePinChg         = true;
        if(pinIdx == FAN_PIN_IDX) 
              haveFanPinChg = true;
        int outPinGpioNum  = ledOutPins[pinIdx];
        u8  outPinLvl      = !inPinLvl;
        outPinLvls[pinIdx] = outPinLvl;
        digitalWrite(outPinGpioNum, outPinLvl);
      }
      else pinChanged[pinIdx] = false;
    }
    bool wsConn    = (wifiEnabled && wsConnected());
    bool wsConnChg = (wsConn != wsWasConnected);
    wsWasConnected = wsConn;

    if(wsConn && (wsConnChg || havePinChg))
        sendPinVals(wsConnChg);

    if(haveFanPinChg) {
      if(!inPinLvls[FAN_PIN_IDX]) {
        // fan turned on -> Y relay stays open
        fanOnTime        = now;
        waitingForYDelay = true;
      } else {  
        // fan turned off -> open Y relay 
        digitalWrite(PIN_OPEN_Y1, HIGH);
        digitalWrite(PIN_OPEN_Y2, HIGH);
      }
    }
  }
  if(waitingForYDelay && ((now - fanOnTime) > yDelayMs)) {
    // Y delay timeout -> close Y relay 
    waitingForYDelay = false;
    digitalWrite(PIN_OPEN_Y1, LOW);
    digitalWrite(PIN_OPEN_Y2, LOW);
  }
}