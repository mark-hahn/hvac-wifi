#ifndef HVAC_H
#define HVAC_H

#include "main.h"

#define SIM_BTN_PRESS_LOW_MS  20   // 20ms
#define SIM_BTN_PRESS_HIGH_MS 40   // 40ms
#define SIM_BTN_EDGE_COUNT    33   // 33*(20ms+40ms)/2 ~ 1s

#define PHY_BTN_TIMEOUT_MS   250

void wsRecv(const u8 *data);

void hvacSetup();
void hvacLoop();

#endif // HVAC_H