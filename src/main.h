#ifndef MAIN_H
#define MAIN_H

#include <arduino.h>
#include <stdint.h>

#define USE_SERIAL
// #define USE_TELNET  // includes SERIAL
// #define SHOW_TIME

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;


/*
  ota, telnet, and stream work together
  sometimes cannot ota while telnet open
  closing telnet kills ota
*/

#if !defined(USE_TELNET) && !defined(USE_SERIAL) 
  // Not PRINTING
  #define prt(...)   do {} while (false)
  #define prts(...)  do {} while (false)
  #define prtl(...)  do {} while (false)
  #define prtf(...)   do {} while (false)
  #define prtfl(...)  do {} while (false)
  #define prtv(...)  do {} while (false)
  #define prtvs(...) do {} while (false)
  #define prtvl(...) do {} while (false)

#else // PRINTING

#ifdef USE_TELNET
  #include <TelnetStream.h>

  #define prt(...)  do{ TelnetStream.print(__VA_ARGS__);   \
                        Serial.print(__VA_ARGS__); }       \
                    while(false)
  #define prtl(...)  do{ TelnetStream.println(__VA_ARGS__); \
                         Serial.println(__VA_ARGS__); }     \
                     while(false)
  #define prtf(...)  do{ TelnetStream.printf(__VA_ARGS__);  \
                         Serial.printf(__VA_ARGS__); }      \
                     while(false)
  #define prtfl(...)  do{ prtf(__VA_ARGS__);                 \
                          TelnetStream.println();            \
                          Serial.println(); }                \
                      while(false)
  #define prtv(...)  do {prt(#__VA_ARGS__);                   \
                         prt(": ");                           \
                         prtl(__VA_ARGS__);}                  \
                      while(false)
  #define prtvs(...) do {prtv(__VA_ARGS__); prt(" ");} while(false)
  #define prtvl(...) do {prt(#__VA_ARGS__);                   \
                         prt(": ");                           \
                         prtl(__VA_ARGS__);}                  \
                      while(false)
#endif // USE_TELNET

#ifdef USE_SERIAL // USE_SERIAL
  #include "Arduino.h"
  #define prt(...)  Serial.print(__VA_ARGS__)
  #define prts(...)  do {Serial.print(__VA_ARGS__); \
                          Serial.print(" ");} \
                      while(false)
  #define prtc(...)  do {Serial.print(__VA_ARGS__); \
                          Serial.print(": ");} \
                      while(false)
  #define prtl(...) Serial.println(__VA_ARGS__)
  #define prtf(...)  Serial.printf(__VA_ARGS__)
  #define prtfl(...)  do {prtf(__VA_ARGS__); Serial.println();} while(false)
  #define prtv(...)  do {prt(#__VA_ARGS__);        \
                        Serial.print(": ");  \
                        Serial.print(__VA_ARGS__);} \
                      while(false)
  #define prtvs(...) do {prtv(__VA_ARGS__); prt(" ");} while(false)
  #define prtvl(...) do {prt(#__VA_ARGS__);             \
                          Serial.print(": ");     \
                          Serial.println(__VA_ARGS__);}  \
                      while(false)
#endif // USE_SERIAL
#endif // PRINTING

#endif // MAIN_H
