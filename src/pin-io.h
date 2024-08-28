#ifndef HVAC_H
#define HVAC_H

#include "main.h"

void getPinStatus(char* res);

void pinIoSetup();
void pinIoLoop();

#endif // HVAC_H