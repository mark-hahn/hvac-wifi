#ifndef WIFI_H
#define WIFI_H

#define DEF_SSID     "hahn-fi"
#define DEF_PASSWORD "90-NBVcvbasd"
// 192.168.1.76

void wsSend(const char *message);

void wifiSetup();
void wifiLoop();

#endif // WIFI_H
