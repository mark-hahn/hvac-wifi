#ifndef HVAC_H
#define HVAC_H

#include "main.h"

void wsRecv(const u8 *data);

void hvacSetup();
void hvacLoop();

#endif // HVAC_H