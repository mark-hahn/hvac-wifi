#ifndef IO_H
#define IO_H

#define BOUNCE_DELAY_MS 50

#define BEEP_CHANNEL       0
#define BEEP_HZ         1000
#define BEEP_LENGTH_MS   150
#define BEEP_RESOLUTION   16
#define MAX_NOTE          12
#define MAX_OCTAVE         8

extern bool buttonPressed;

// NOTE_C, NOTE_Cs, NOTE_D, NOTE_Eb, NOTE_E, NOTE_F, 
// NOTE_Fs, NOTE_G, NOTE_Gs, NOTE_A, NOTE_Bb, NOTE_B, NOTE_MAX
void beepNote(note_t note, uint8_t octave);
void beepHz(uint32_t noteFreq);
void beeps(u8 beepCount); 
void beepOff();

void powerOff();

void ioSetup();
void ioLoop();

#endif // IO_H
