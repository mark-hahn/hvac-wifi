#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "main.h"
#include "pins.h"
#include "pin-io.h"
#include "wifi-sta.h"

bool wifiConnected = false;

char ssid[]     = DEF_SSID;
char password[] = DEF_PASSWORD;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

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

void printMacAddr(){
  u8 baseMac[6];
  WiFi.macAddress(baseMac);
  prtfl("mac: %02x:%02x:%02x:%02x:%02x:%02x",
              baseMac[0], baseMac[1], baseMac[2],
              baseMac[3], baseMac[4], baseMac[5]);
}

int wsConnCount() {
  return ws.count();
}

void wsSendMsg(const char * message) {
  setWifiLedPulsing(true, true); // one pulse
  ws.textAll(message);
}

void wsRecvMsg(void *arg, u8 *data, size_t len) {
  setWifiLedPulsing(true, true); // one pulse
  AwsFrameInfo *info = (AwsFrameInfo*) arg;
  if (info->final && info->index == 0 && info->len == len 
                  && info->opcode == WS_TEXT) {
    data[len] = 0;
    const char* msg = (const char*) data;
    if (!strcmp(msg, "query")) 
      // query cmd sends all pin vals
      sendPinVals(true);
    else if(msg[0] >= '0' && msg[0] <= '9') 
      // numerical messages are delay in ms
      yDelayMs = atoi(msg);
    else
      prtfl("unsupported msg recvd: %s", msg);
  }
}

void eventHandler(AsyncWebSocket *server, 
                  AsyncWebSocketClient *client, AwsEventType type, 
                  void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", 
                    client->id(), 
                    client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      wsRecvMsg(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      prtfl("%s %d", "WS_EVT_UNKNOWN", (int) type);
      break;
  }
}

void wifiSetup() {
  WiFi.begin(ssid, password);

  ws.onEvent(eventHandler);
  server.addHandler(&ws);
  server.begin();

  setWifiLedPulsing(true, false);
}

void wifiLoop() {
  static bool oldWsConnCnt = 0;
  static u32 lastDotTime = 0;
  bool now = millis();

  ws.cleanupClients();

  if (WiFi.status() == WL_CONNECTED) {
    if(!wifiConnected) {
      wifiConnected = true;
      prt("Connected to WiFi: ");
      prtl(WiFi.localIP());
      printMacAddr();
      setWifiLedPulsing(false, false);
    }
  }
  else {
    if(wifiConnected) {
      wifiConnected = false;
      prtl();
      prtl("Disconnected from WiFi");
      WiFi.begin(ssid, password);
      setWifiLedPulsing(true, false);
    }
    if ((now - lastDotTime) > 1000) {
      prt(".");
      lastDotTime = now;
    }
  }

  int wsConnCnt = ws.count();
  if(wsConnCnt != oldWsConnCnt) {
    prtfl("new wsConnCnt: %d", wsConnCnt);
    oldWsConnCnt = wsConnCnt;
    if(wsConnCnt) sendPinVals(true);
  }

  static u32 lastPingTime = 0;
  if ((now - lastPingTime) > PING_INTERVAL) {
    if(ws.count()) {
      prtl("pinging all");
      wsSendMsg((const char*) "ping");
    }
    lastPingTime = now;
  }

  checkWifiLed();
}
