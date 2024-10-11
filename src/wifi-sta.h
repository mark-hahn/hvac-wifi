#ifndef WIFI_H
#define WIFI_H

// FIXED IP IS DEFINED IN ROUTER
// MAC is c8:2e:18:f0:06:2c
// IP  is 192.168.1.106

// keep Y1D relay open permanently?
extern bool disableY1d;

#define DEF_SSID     "hahn-fi"
#define DEF_PASSWORD "90-NBVcvbasd"

#define PING_INTERVAL 10000  // ping every 10 secs

int  wsConnCount();
void wsSendMsg(const char *message);

void wifiSetup();
void wifiLoop();

#endif // WIFI_H
