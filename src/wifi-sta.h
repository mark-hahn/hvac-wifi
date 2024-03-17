#ifndef WIFI_H
#define WIFI_H

#define DEF_SSID     "hahn-fi"
#define DEF_PASSWORD "90-NBVcvbasd"
// 192.168.1.79

#define PING_INTERVAL 10000  // ping every 10 secs

extern bool wifiConnected;

bool wsConnected();
void wsSend(const char *message);

void wifiSetup();
void wifiLoop();

#endif // WIFI_H
