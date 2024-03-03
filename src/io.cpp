#include <arduino.h>
#include <esp32-hal-ledc.h>
#include "main.h"
#include "pins.h"
#include "camera.h"
#include "wifi-sta.h"
#include "io.h"

void powerOff() {
  prtl("Powering off");
  digitalWrite(POWER_GPIO_NUM, LOW);
  while(true) yield();
}

bool buttonPressed = false;

void handleButtonDown() {
  prtl("Button down");
  buttonPressed = true;
}

void handleButtonUp() {
  // prtl("Button up");
}

void buttonLoop() {
  static u32 lastChange = 0;
  static u32 buttonDown = false;

  if((millis() - lastChange) > BOUNCE_DELAY_MS) {
    if(!buttonDown) {
      if(digitalRead(BUTTON_GPIO_NUM) == HIGH) {
        buttonDown = true;
        lastChange = millis();
        handleButtonDown();
      }
    } else {
      if(digitalRead(BUTTON_GPIO_NUM) == LOW) {
        buttonDown = false;
        lastChange = millis();
        handleButtonUp();
      }
    }
  }
}

u8 beepCounter = 0;

void beeps(u8 beepCount) {
  // prtfl("Beeping %d times", beepCount);
  beepCounter = 2 * beepCount;
}
void beepLoop() {
  static u32  lastBeep = 0;
  static bool beepOn   = false;

  if(beepCounter > 0) {
    if((millis() - lastBeep) > BEEP_LENGTH_MS) {
      // prtfl("%d Beep counter: %d, on: %d", millis(), beepCounter, beepOn);
      lastBeep = millis();
      beepOn = !beepOn;
      if(beepOn) {
        if(beepCounter == 0) {
          beepOff();
          beepOn = false;
        }
        else
          beepHz(BEEP_HZ);
      }
      else {
        beepOff();
      }
      beepCounter--;
    }
  }
}
void beepNote(note_t note, uint8_t octave) {
  // prtfl("Beeping note %d, octave %d", note, octave);
  ledcWriteNote(BEEP_CHANNEL, note, octave);
}
void beepHz(uint32_t noteFreq) {
  // prtfl("BeepHz freq %d", noteFreq);
  ledcWriteTone(BEEP_CHANNEL, noteFreq);
}
void beepOff() {
  // prtl("Beep off");
  ledcWrite(BEEP_CHANNEL, 0);
}

void ioSetup() {
  ledcSetup(BEEP_CHANNEL, BEEP_HZ, BEEP_RESOLUTION);
  ledcAttachPin(BEEP_GPIO_NUM, BEEP_CHANNEL);
}

void ioLoop() {
  buttonLoop();
  beepLoop();
}
