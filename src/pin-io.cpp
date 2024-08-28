#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "pins.h"

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

void getPinStatus(char* res){
  char res[64] = "{";
  for (u8 pinIdx = 0; pinIdx < NUM_IN_PINS; pinIdx++) {
    u8 pin           = pins[pinIdx];
    u8 pinLvl        = digitalRead(pin);
    lastPinLvl[pin]  = pinLvl;
    const char *name = pinToName(pin);
    char msg[16];
    if (pinLvl) sprintf(msg, "\"%s\":false,", name);
    else        sprintf(msg, "\"%s\":true,",  name);
    strcat(res, msg);
  }
  res[strlen(res) - 1] = '}';

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
  static u8  inPinLvls[sizeof ledInPins]  = {255};
  static u8 outPinLvls[sizeof ledOutPins] = {255};
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
      bool haveFanPinChg = false;
      for(int pinIdx = 0; 
          pinIdx < (sizeof ledInPins);  pinIdx++) {
        int inPinGpioNum  = ledInPins[pinIdx];
        u8  inPinLvl      = digitalRead(inPinGpioNum);
        inPinLvls[pinIdx] = inPinLvl;
        if (inPinLvl == outPinLvls[pinIdx]) {
          // input pin changed
          int outPinGpioNum = ledOutPins[pinIdx];
          if(pinIdx == FAN_PIN_IDX) haveFanPinChg = true;
          u8 outPinLvl       = !inPinLvl;
          outPinLvls[pinIdx] = outPinLvl;
          digitalWrite(outPinGpioNum, outPinLvl);
        }
      }
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