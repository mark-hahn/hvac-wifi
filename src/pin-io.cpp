#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "pins.h"

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

u8 ledOutPins[] = {
  PIN_LED_Y1  ,
  PIN_LED_Y1D ,
  PIN_LED_Y2  ,
  PIN_LED_Y2D ,
  PIN_LED_G   ,
  PIN_LED_W1  ,
  PIN_LED_W2  ,
  PIN_LED_PWR
};

#define UNKNOWN_LVL 255
u8 ledInPinLvls  [sizeof ledInPins]  = {UNKNOWN_LVL};
u8 ledOutPinLvls [sizeof ledOutPins] = {UNKNOWN_LVL};

void readLedPins() {

}

void writeLedPins() {

}

void getPinStatus(char* res){
  char res[64] = "{";
  for (u8 i = 0; i < NUM_IN_PINS; i++) {
    u8 pin           = pins[i];
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

void pinIoSetup() {
  for(int i = 0; i < sizeof ledInPins;  i++)
    pinMode(ledInPins[i],  INPUT);
  for(int i = 0; i < sizeof ledOutPins; i++)
    pinMode(ledOutPins[i], OUTPUT);
  pinMode(PIN_ENBL_WIFI,   INPUT);
  pinMode(PIN_OPEN_Y1,     OUTPUT);  
  pinMode(PIN_OPEN_Y2,     OUTPUT);    
}

void pinIoLoop() {

}