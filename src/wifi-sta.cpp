#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "main.h"
#include "wifi-sta.h"

char ssid[]     = DEF_SSID;
char password[] = DEF_PASSWORD;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*) arg;
  if (info->final && info->index == 0 && info->len == len 
                  && info->opcode == WS_TEXT) {
    data[len] = 0;
    prtl((char*)data);
  }
}

void eventHandler(AsyncWebSocket *server, 
                  AsyncWebSocketClient *client, AwsEventType type, 
                  void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", 
                    client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      prtl("\ngot WS_EVT_DATA");
      handleWebSocketMessage(arg, data, len);
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

  ws.cleanupClients();
}
