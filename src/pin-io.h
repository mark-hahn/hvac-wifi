#ifndef PINIO_H
#define PINIO_H

#define DEBOUNCE_DELAY_MS 2
#define MAX_SAMPLE_DELAY  5

#define DEFAULT_YDELAY_MS 15000
#define WIFI_LED_PULSE_MS 250

#define POWER_LOSS_TIMEOUT 500

extern int yDelayMs;
void sendPinVals(bool forceAll);

void pinIoSetup();
void pinIoLoop();

#endif // PINIO_H