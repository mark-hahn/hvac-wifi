// https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card

#include "Arduino.h"
#include "soc/soc.h"           // soc reg for brownout
#include "soc/rtc_cntl_reg.h"  // rtc pins for brownout
#include "esp32/rom/rtc.h"     // reset reason

#include "main.h"
#include "control.h"
#include "pins.h"
#include "wifi-sta.h"
#include "camera.h"
#include "accel.h"
#include "store.h"
#include "io.h"

void verbose_print_reset_reason(int reason) {
  if(reason < 2) return;
  prt("Reset reason: ");
  switch (reason) {
    case 1  : prtl("Vbat power on reset");break;
    case 3  : prtl("Software reset digital core");break;
    case 4  : prtl("Legacy watch dog reset digital core");break;
    case 5  : prtl("Deep Sleep reset digital core");break;
    case 6  : prtl("Reset by SLC module, reset digital core");break;
    case 7  : prtl("Timer Group0 Watch dog reset digital core");break;
    case 8  : prtl("Timer Group1 Watch dog reset digital core");break;
    case 9  : prtl("RTC Watch dog Reset digital core");break;
    case 10 : prtl("Instrusion tested to reset CPU");break;
    case 11 : prtl("Time Group reset CPU");break;
    case 12 : prtl("Software reset CPU");break;
    case 13 : prtl("RTC Watch dog Reset CPU");break;
    case 14 : prtl("for APP CPU, reseted by PRO CPU");break;
    case 15 : prtl("Reset when the vdd voltage is not stable");break;
    case 16 : prtl("RTC Watch dog reset core and rtc");break;
    default : prtfl("Bad reset reason: %d", reason);
  }
}

void setup() {
  // disableBrownout();

  pinMode(FLASH_GPIO_NUM, OUTPUT); // flash
  pinMode(PWDN_GPIO_NUM,  OUTPUT); // camera power

  cameraOn();
  flashOn();

#ifdef USE_SERIAL
  Serial.begin(921600);
  // while (!Serial);
  // sleep(2); // debug, give monitor time to start
  prtl("\n\nStarting...");
#endif

  verbose_print_reset_reason(rtc_get_reset_reason(0));
  verbose_print_reset_reason(rtc_get_reset_reason(1));

  flashOff();

  wifiSetup();
  cameraSetup();
  storeSetup();
  ioSetup();
  accelSetup();

  prtfl("All setup complete, millis: %d\n", millis());
  beeps(1);
}

void loop() {
  // if(millis() > 5000) {
  //   prtl("Going to sleep");
  //   esp_sleep_enable_ext0_wakeup(
  //       (gpio_num_t) BUTTON_GPIO_NUM, HIGH);
  //   esp_deep_sleep_start();
  // }
  wifiLoop();
  cameraLoop();
  ioLoop();
  accelLoop();
  controlLoop();
}