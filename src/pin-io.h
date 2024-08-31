#ifndef PINIO_H
#define PINIO_H

#define DEBOUNCE_DELAY_MS 2
#define PWR_ACTV_MS       6

#define DEFAULT_YDELAY_MS 10000
#define WIFI_LED_PULSE_MS 250

extern int yDelayMs;
void sendPinVals(bool forceAll);

void pinIoSetup();
void pinIoLoop();

#endif // PINIO_H