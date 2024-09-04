#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "wifi-sta.h"
#include "pins.h"
#include "pin-io.h"

u8 inputGpios[] = {
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
#define PWR_PIN_IDX 7
#define PIN_COUNT (sizeof inputGpios)

u8 ledOutGpios[PIN_COUNT] = {
  PIN_LED_Y1 ,
  PIN_LED_Y1D,
  PIN_LED_Y2 ,
  PIN_LED_Y2D,
  PIN_LED_G  ,
  PIN_LED_W1 ,
  PIN_LED_W2 ,
  PIN_LED_PWR
};

const char* pinNames[PIN_COUNT] = {
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

u8   inPinLvls[PIN_COUNT]  = {1};
u8   outPinLvls[PIN_COUNT] = {0};
bool pinChanged[PIN_COUNT] = {false};

// send pin status to server when any pin changes
void sendPinVals(bool forceAll) {
  char json[128];
  json[0] = '{';
  json[1] = 0;
  for (int pinIdx = 0; pinIdx < PIN_COUNT; pinIdx++) {
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
  if(wifiEnabled) wsSendMsg(json);
}

u32 lastPwrPulseTime;
u32 lastPwrFallTime = 0;

void IRAM_ATTR handlePowerPinFall() {
  detachInterrupt(PIN_IN_PWR);
  lastPwrFallTime = millis();
}


void pinIoSetup() {
  for(int pinIdx = 0; pinIdx < PIN_COUNT;  pinIdx++) {
    pinMode(inputGpios[pinIdx], INPUT);
    int gpioNum = ledOutGpios[pinIdx];
    pinMode(gpioNum, OUTPUT);
    digitalWrite(gpioNum, LOW);
  }
  pinMode(PIN_ENBL_WIFI, INPUT);
  pinMode(PIN_LED_WIFI,  OUTPUT);
  pinMode(PIN_OPEN_Y1,   OUTPUT);  
  pinMode(PIN_OPEN_Y2,   OUTPUT);  

  digitalWrite(PIN_LED_WIFI, LOW);
  digitalWrite(PIN_OPEN_Y1,  LOW);
  digitalWrite(PIN_OPEN_Y2,  LOW);
  attachInterrupt(PIN_IN_PWR, handlePowerPinFall, FALLING);  

  lastPwrPulseTime = millis();
}

void pinIoLoop() {
  static u32  fanOnTime        = 0;
  static bool waitingForYDelay = false;
  static bool wsWasConnected   = false;
  u32 now = millis();

  u32 pwrDelay = now - lastPwrFallTime;
  if(lastPwrFallTime && pwrDelay >= DEBOUNCE_DELAY_MS) {
    static u32 lastPwrErrorTime = 0;
    lastPwrPulseTime = now;

    // interrupt happened, inside power pulse?
    if(pwrDelay <= MAX_SAMPLE_DELAY) {
      // all pin inputs valid, read pins asap
      for(int pinIdx = 0; pinIdx < PIN_COUNT;  pinIdx++)
        inPinLvls[pinIdx] = digitalRead(inputGpios[pinIdx]);

      // power pin high sanity check
      if(inPinLvls[PWR_PIN_IDX] && 
          (now - lastPwrErrorTime) > 15000) {
        prtl("error: power pin high");
        if(wifiEnabled) wsSendMsg("error: power pin high");
        lastPwrErrorTime = now;
      }
      else { 
        // power pulse is valid
        bool havePinChg    = false;
        bool haveFanPinChg = false;
        u8   fanPinInLvl;

        // get outPinLvls & check for pin changes
        for(int pinIdx = 0; pinIdx < PIN_COUNT;  pinIdx++) {
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
        for(int pinIdx = 0; pinIdx < PIN_COUNT;  pinIdx++) {
          u8 inPinLvl = inPinLvls[pinIdx];
          int gpioNum     = ledOutGpios[pinIdx];
          u8  outPinLvl   = outPinLvls[pinIdx];
          u8  outWriteVal = outPinLvl;
          if(pinIdx == Y1_PIN_IDX && outPinLvl && 
              outPinLvls[Y1D_PIN_IDX])
            outWriteVal = 0;
          if(pinIdx == Y2_PIN_IDX && outPinLvl && 
              outPinLvls[Y2D_PIN_IDX])
            outWriteVal = 0;
          digitalWrite(gpioNum, outWriteVal);
        }

        bool wsConn    = (wifiEnabled && wsConnCount());
        bool wsConnChg = (wsConn != wsWasConnected);
        wsWasConnected = wsConn;

        // send changed pin values out wifi
        // if new web socket then send all pins
        if(wsConn && (wsConnChg || havePinChg))
          sendPinVals(wsConnChg);

        if(haveFanPinChg) {
          if(!fanPinInLvl) {
            // fan turned on -> Y relays stay open
            fanOnTime        = now;
            waitingForYDelay = true;
          } else {  
            // fan turned off -> open Y relays
            digitalWrite(PIN_OPEN_Y1, HIGH);
            digitalWrite(PIN_OPEN_Y2, HIGH);
          }
        }
      }
    }
    lastPwrFallTime = 0;
    attachInterrupt(PIN_IN_PWR, handlePowerPinFall, FALLING); 
  }
  
  if(waitingForYDelay && ((now - fanOnTime) > yDelayMs)) {
    // Y delay timeout -> close Y relays
    waitingForYDelay = false;
    digitalWrite(PIN_OPEN_Y1, LOW);
    digitalWrite(PIN_OPEN_Y2, LOW);
  }

  if((now - lastPwrPulseTime) > POWER_LOSS_TIMEOUT) {
    lastPwrPulseTime = now;
    for(int pinIdx = 0; pinIdx < PIN_COUNT;  pinIdx++) {
      ledOutGpios[pinIdx] = 0;
      digitalWrite(ledOutGpios[pinIdx], LOW);
    }
  }
}