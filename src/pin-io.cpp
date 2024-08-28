#include <Arduino.h>
#include <WiFi.h>

#include "pin-io.h"
#include "main.h"
#include "pins.h"
#include "wifi-sta.h"

#define Y_DELAY_MS 5000

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

void readLedPins() {

}

void writeLedPins() {

}

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

u8  inPinLvls[sizeof ledInPins]  = {255};
u8 outPinLvls[sizeof ledOutPins] = {255};

// send pin status to server when any pin changes
void sendPinStatus() {
  char json[128];
  json[0] = '{';
  json[1] = 0;
  for (int pinIdx = 0; pinIdx < (sizeof ledOutPins); pinIdx++) {
    const char *name = pinToName(pinIdx);
    u8 pinLvl = outPinLvls[pinIdx];
    char msg[16];
    if (pinLvl) sprintf(msg, "\"%s\":true,",   name);
    else        sprintf(msg, "\"%s\":false,",  name);
    strcat(json, msg);
  }
  json[strlen(json) - 1] = '}';
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
  attachInterrupt(
    PIN_IN_PWR, handlePowerPinFall, FALLING);  
  digitalWrite(PIN_OPEN_Y1, LOW);
  digitalWrite(PIN_OPEN_Y2, LOW);
}

void pinIoLoop() {
  static u32  fanOnTime                   = 0;
  static bool waitingForFanDelay          = false;

  u32 now = millis();

  if(lastPwrFallMs) {
    if((now - lastPwrFallMs) > 2) {
      lastPwrFallMs = 0;
      // power fall debounced

      attachInterrupt(
        PIN_IN_PWR, handlePowerPinFall, FALLING); 

      // middle of power pulse, all pin inputs valid
      bool havePinChg    = false;
      bool haveFanPinChg = false;
      for(int pinIdx = 0; 
          pinIdx < (sizeof ledInPins);  pinIdx++) {
        int inPinGpioNum  = ledInPins[pinIdx];
        u8  inPinLvl      = digitalRead(inPinGpioNum);
        inPinLvls[pinIdx] = inPinLvl;
        if (inPinLvl == outPinLvls[pinIdx]) {
          // input pin changed
          havePinChg = true;
          if(pinIdx == FAN_PIN_IDX) haveFanPinChg = true;
          int outPinGpioNum  = ledOutPins[pinIdx];
          u8 outPinLvl       = !inPinLvl;
          outPinLvls[pinIdx] = outPinLvl;
          digitalWrite(outPinGpioNum, outPinLvl);
        }
      }
      if(havePinChg) sendPinStatus();
      if(haveFanPinChg) {
        if(!inPinLvls[FAN_PIN_IDX]) {
          // fan turned on -> Y relay stays open
          fanOnTime          = now;
          waitingForFanDelay = true;
        } else {  
          // fan turned off -> open Y relay 
          digitalWrite(PIN_OPEN_Y1, HIGH);
          digitalWrite(PIN_OPEN_Y2, HIGH);
        }
      }
    }
  }
  if(waitingForFanDelay && 
      (now - fanOnTime) > Y_DELAY_MS) {
    // fan delay timeout -> close Y relay 
    waitingForFanDelay = false;
    digitalWrite(PIN_OPEN_Y1, LOW);
    digitalWrite(PIN_OPEN_Y2, LOW);
  }
}