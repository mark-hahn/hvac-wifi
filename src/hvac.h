#ifndef HVAC_H
#define HVAC_H

#include "main.h"

#define BTN_PRESS_LOW_MS  20   // 20ms
#define BTN_PRESS_HIGH_MS 40   // 40ms
#define BTN_EDGE_COUNT    30   // 30 * 33ms = 1s
#define BTN_TIME_MS      500   // 500ms

void wsRecv(const u8 *data);

void hvacSetup();
void hvacLoop();

#endif // HVAC_H