#include <Arduino.h>
#include <WiFi.h>

#include "pin-io.h"
#include "main.h"
#include "pins.h"
#include "wifi-sta.h"

u8 inputPinGpios[] = {
  PIN_IN_Y1 ,   
  PIN_IN_Y1D,   
  PIN_IN_Y2 ,   
  PIN_IN_Y2D,   
  PIN_IN_G  ,   
  PIN_IN_W1 ,   
  PIN_IN_W2 ,   
  PIN_IN_PWR
};

#define Y1_PIN_IDX  0
#define Y1D_PIN_IDX 1
#define Y2_PIN_IDX  2
#define Y2D_PIN_IDX 3
#define FAN_PIN_IDX 4

u8 ledOutPinGpios[sizeof inputPinGpios] = {
  PIN_LED_Y1 ,
  PIN_LED_Y1D,
  PIN_LED_Y2 ,
  PIN_LED_Y2D,
  PIN_LED_G  ,
  PIN_LED_W1 ,
  PIN_LED_W2 ,
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

u8   inPinLvls[sizeof inputPinGpios]   = {1};
u8   outPinLvls[sizeof ledOutPinGpios] = {0};
bool pinChanged[sizeof inputPinGpios]  = {false};

// send pin status to server when any pin changes
void sendPinVals(bool forceAll) {
  char json[128];
  json[0] = '{';
  json[1] = 0;
  for (int pinIdx = 0; pinIdx < pinCount; pinIdx++) {
    if(forceAll || pinChanged[pinIdx]) {
      const char* name = pinNames[pinIdx];
      u8 pinLvl        = outPinLvls[pinIdx];
      char msg[32];
      if (pinLvl) sprintf(msg, "\"%s\":1,", name);
      else        sprintf(msg, "\"%s\":0,", name);
      strcat(json, msg);
    }
  }
  json[strlen(json)-1] = '}';
  wsSendMsg(json);
}

u32 lastPwrFallTime = 0;

void IRAM_ATTR handlePowerPinFall() {
  detachInterrupt(PIN_IN_PWR);
  lastPwrFallTime = millis();
}

bool wifiLedPulsing = false;
bool wifiLedOnce    = false;
u32  wifiLedChgTime = 0;
bool wifiLedOn      = false;

void setWifiLedPulsing(bool pulsing, bool once) {
  wifiLedPulsing = pulsing;
  wifiLedOnce    = once;    
  if(wifiLedPulsing) {
    wifiLedChgTime = millis();
    wifiLedOn      = false;
    digitalWrite(PIN_LED_WIFI, LOW);
  }
  else {
    wifiLedOn = true;
    digitalWrite(PIN_LED_WIFI, HIGH);
  }
}

void checkWifiLed() {
  if(!wifiLedPulsing) return;
  u32 now = millis();
  if((now - wifiLedChgTime) > WIFI_LED_PULSE_MS) {
    wifiLedChgTime = now;
    wifiLedOn      = !wifiLedOn;
    digitalWrite(PIN_LED_WIFI, wifiLedOn);
    if(wifiLedOnce && wifiLedOn)
      wifiLedPulsing = false;
  }
}

void pinIoSetup() {
  for(int pinIdx = 0; pinIdx < pinCount;  pinIdx++)
    pinMode(inputPinGpios[pinIdx],  INPUT);
  for(int pinIdx = 0; pinIdx < pinCount; pinIdx++) {
    pinMode(ledOutPinGpios[pinIdx], OUTPUT);
    digitalWrite(ledOutPinGpios[pinIdx], LOW);
  }
  pinMode(PIN_ENBL_WIFI, INPUT);
  pinMode(PIN_LED_WIFI,  OUTPUT);
  pinMode(PIN_OPEN_Y1,   OUTPUT);  
  pinMode(PIN_OPEN_Y2,   OUTPUT);  

  digitalWrite(PIN_LED_WIFI, LOW);
  digitalWrite(PIN_OPEN_Y1,  LOW);
  digitalWrite(PIN_OPEN_Y2,  LOW);

  wifiLedOn = false;

  attachInterrupt(
    PIN_IN_PWR, handlePowerPinFall, FALLING);  
}

void pinIoLoop() {
  static u32  fanOnTime        = 0;
  static bool waitingForYDelay = false;
  static bool wsWasConnected   = false;
  u32 now      = millis();
  int pinCount = (sizeof inputPinGpios);

  if(lastPwrFallTime) {
    // interrupt happened, start of power pulse

    u32 pwrDelay = now - lastPwrFallTime;
    if(pwrDelay > DEBOUNCE_DELAY_MS &&
       pwrDelay < PWR_ACTV_MS) {

      // all pin inputs valid, read pins asap
      for(int pinIdx = 0; pinIdx < pinCount;  pinIdx++)
        inPinLvls[pinIdx] = digitalRead(inputPinGpios[pinIdx]);
      
      bool havePinChg    = false;
      bool haveFanPinChg = false;
      u8   fanPinInLvl;

      // get outPinLvls & check for pin changes
      for(int pinIdx = 0; pinIdx < pinCount;  pinIdx++) {
        u8 inPinLvl = inPinLvls[pinIdx];
        // compare in to old out
        if (inPinLvl == outPinLvls[pinIdx]) {
          pinChanged[pinIdx] = true;
          havePinChg         = true;
          if(pinIdx == FAN_PIN_IDX) {
            haveFanPinChg = true;
            fanPinInLvl   = inPinLvl;
          }
        } else 
          pinChanged[pinIdx] = false;
        outPinLvls[pinIdx] = !inPinLvl;
      }

      // digitalWrite all out pins
      for(int pinIdx = 0; pinIdx < pinCount;  pinIdx++) {
        u8 inPinLvl = inPinLvls[pinIdx];
        int gpioNum     = ledOutPinGpios[pinIdx];
        u8  outPinLvl   = outPinLvls[pinIdx];
        u8  outWriteVal = outPinLvl;
        if(pinIdx == Y1_PIN_IDX && outPinLvl && 
            outPinLvls[Y1D_PIN_IDX] )
          outWriteVal = 0;
        if(pinIdx == Y2_PIN_IDX && outPinLvl && 
            outPinLvls[Y2D_PIN_IDX] )
          outWriteVal = 0;
        digitalWrite(gpioNum, outWriteVal);
      }

      bool wsConn    = (wifiEnabled && wsConnected());
      bool wsConnChg = (wsConn != wsWasConnected);
      wsWasConnected = wsConn;

      // send changed pin values out wifi
      if(wsConn && (wsConnChg || havePinChg))
        sendPinVals(wsConnChg);

      if(haveFanPinChg) {
        if(!fanPinInLvl]) {
          // fan turned on -> Y relays stay open
          fanOnTime        = now;
          waitingForYDelay = true;
        } else {  
          // fan turned off -> open Y relay s
          digitalWrite(PIN_OPEN_Y1, HIGH);
          digitalWrite(PIN_OPEN_Y2, HIGH);
        }
      }
      inPwrPulses++; // inside power pulse
    }
    pwrPulses++;
    lastPwrFallTime = 0;
    attachInterrupt(PIN_IN_PWR, handlePowerPinFall, FALLING); 
  }
  if(waitingForYDelay && ((now - fanOnTime) > yDelayMs)) {
    // Y delay timeout -> close Y relays
    waitingForYDelay = false;
    digitalWrite(PIN_OPEN_Y1, LOW);
    digitalWrite(PIN_OPEN_Y2, LOW);
  }
  if(wifiEnabled) checkWifiLed();
}