#ifndef PINIO_H
#define PINIO_H

void sendChangedPins(bool forceAll);

void pinIoSetup();
void pinIoLoop();

#endif // PINIO_H