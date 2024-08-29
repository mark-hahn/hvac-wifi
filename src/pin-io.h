#ifndef PINIO_H
#define PINIO_H

void sendPinVals(bool forceAll);
void setYDelay(int delay);
void setWifiLedPulsing(bool pulsing, bool once = false);

void pinIoSetup();
void pinIoLoop();

#endif // PINIO_H