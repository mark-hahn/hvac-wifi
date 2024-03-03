#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "main.h"
#include "wifi-sta.h"

char ssid[]     = DEF_SSID;
char password[] = DEF_PASSWORD;

void wifiSetup() {
  prtf("Using SSID: %s, Password: %s\n", ssid, password);  
  WiFi.begin(ssid, password);
}

void wifiLoop() {
  static u32 lastMillis = 0;
  static bool wifiConnected = false;
  if (WiFi.status() != WL_CONNECTED) {
    if(wifiConnected) {
      wifiConnected = false;
      prtl();
      prtl("Disconnected from WiFi");
      WiFi.begin(ssid, password);
    }
    if ((millis() - lastMillis) > 1000) {
      prt(".");
      lastMillis = millis();
    }
  }
  else {
    if(!wifiConnected) {
      wifiConnected = true;
      prt("Connected to WiFi: ");
      prtl(WiFi.localIP());
    }
  }
}
