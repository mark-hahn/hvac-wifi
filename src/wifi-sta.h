#ifndef WIFI_H
#define WIFI_H

#define DEF_SSID     "hahn-fi"
#define DEF_PASSWORD "90-NBVcvbasd"

#define PING_INTERVAL 10000  // ping every 10 secs

bool wsConnected();
void wsSendMsg(const char *message);

void wifiSetup();
void wifiLoop();

#endif // WIFI_H
