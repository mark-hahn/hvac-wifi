#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "main.h"
#include "pin-io.h"
#include "wifi-sta.h"

bool wifiConnected = false;

char ssid[]     = DEF_SSID;
char password[] = DEF_PASSWORD;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void printMacAddr(){
  u8 baseMac[6];
  WiFi.macAddress(baseMac);
  prtfl("mac: %02x:%02x:%02x:%02x:%02x:%02x",
              baseMac[0], baseMac[1], baseMac[2],
              baseMac[3], baseMac[4], baseMac[5]);
}

bool wsConnected() {
  if(!wifiConnected) return false;
  return (ws.count() > 0);
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
    if (!strcmp(msg, "query")) sendPinVals(true);
    else if(msg[0] >= '0' && msg[0] <= '9') {
      // numerical messages are delay in ms
      setYDelay(atoi(msg));
    }
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
  static bool wsWasConnected = false;
  ws.cleanupClients();

  static u32 lastMillis = 0;
  if (WiFi.status() != WL_CONNECTED) {
    if(wifiConnected) {
      wifiConnected = false;
      prtl();
      prtl("Disconnected from WiFi");
      WiFi.begin(ssid, password);
      setWifiLedPulsing(true, false);
    }
    if ((millis() - lastMillis) > 1000) {
      prt(".");
      lastMillis = millis();
    }
  }
  else {
    if(!wifiConnected) {
      wifiConnected = true;
      prtfl("Connected to WiFi: %s", WiFi.localIP());
      printMacAddr();
      setWifiLedPulsing(false, false);
    }
  }

  bool wsIsConnected = wsConnected();
  if(wsIsConnected != wsWasConnected) {
    prtfl("wsIsConnected changed to %s",
           wsIsConnected ? "true" : "false");
    wsWasConnected = wsIsConnected;
    if(wsIsConnected) sendPinVals(true);
  }

  static u32 lastPingTime = 0;
  if ((millis() - lastPingTime) > PING_INTERVAL) {
    if(wsConnected()) {
      prtl("pinging all");
      wsSendMsg((const char*) "ping");
      // ws.pingAll((u8*) "x", 1);
    }
    lastPingTime = millis();
  }
}
