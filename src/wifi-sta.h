#ifndef WIFI_STA_H
#define WIFI_STA_H

#define DEF_SSID     "hahn-fi"
#define DEF_PASSWORD "90-NBVcvbasd"

extern bool wifiConnected;
extern bool haveSsidPwd;
extern char ssid[64];
extern char password[64];

bool sendJpeg(const char *url, uint8_t *data, size_t len);

void wifiSetup();
void wifiLoop();

#endif // WIFI_STA_H
