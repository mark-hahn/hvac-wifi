#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "main.h"
#include "store.h"
#include "wifi-sta.h"

bool wifiConnected = false;
bool haveSsidPwd   = false;
char ssid[64];
char password[64];

bool sendJpeg(const char *url, u8 *data, size_t len) {
  HTTPClient http;
  prtf("sending jpeg: %s, len: %d\n", url, len);
  if(!wifiConnected) {
    prtl("sendJpeg error: WiFi not connected!");
    return false;
  }
  http.begin(url);
  http.addHeader("Content-Type", "image/jpeg");
  char lenStr[16];
  sprintf(lenStr, "%d", len);
  http.addHeader("Content-Length", lenStr);
  int res = http.POST(data, len);
  if (res > 0) {
    prtf("response code: %d\n", res);
  } else {
    prtf("post error: %s\n", http.errorToString(res).c_str());
    http.end();
  }
  String response = http.getString();
  // prtl("response: " + response);
  http.end();
  return (res > 0);
}

void wifiSetup() {
  haveSsidPwd = getSsidPwd(ssid, password);
  if (!haveSsidPwd) {
    strcpy(ssid,     DEF_SSID);
    strcpy(password, DEF_PASSWORD);
  }
  prtf("Using SSID: %s, Password: %s\n", ssid, password);  
  WiFi.begin(ssid, password);
}

void wifiLoop() {
  static u32 lastMillis = 0;
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
