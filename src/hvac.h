#ifndef HVAC_H
#define HVAC_H

#include "main.h"

#define BTN_TIMEOUT 500   // 500ms
#define NUM_PRESS_30MS 33 // 33 * 30ms = 1s

void hvacRecv(const u8 *data);

void hvacSetup();
void hvacLoop();

#endif // HVAC_H