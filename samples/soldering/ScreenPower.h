#ifndef SCREEN_POWER_H
#define SCREEN_POWER_H

#include "Screen.h"
#include "Iron.h"
#include "Encoder.h"
#include "Display.h"
#include "oo.h"

#define SELF ScreenPower *const self
typedef struct ScreenPowerStruct {
    Screen parent;
    Iron* pIron;                          // Pointer to the iron instance
    Display* pD;                         // Pointer to the display instance
    Encoder* pEnc;                        // Pointer to the rotary encoder instance
    uint32_t update_screen;               // Time in ms to update the screen
    bool on;                              // Weither the power of soldering iron is on
    History hTemp;
    byte max_power;
} ScreenPower;

void createScreenPower(SELF, Iron* iron, Display* display, Encoder *encoder);
void initScreenPower(VOID_SELF);
void showScreenPower(VOID_SELF);
void rotaryValueScreenPower(VOID_SELF, int16_t value);
void *menuScreenPower(VOID_SELF);
void *menu_longScreenPower(VOID_SELF);

#undef SELF
#endif /* SCREEN_POWER_H */
