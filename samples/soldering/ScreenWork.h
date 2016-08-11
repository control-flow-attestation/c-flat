#ifndef SCREEN_WORK_H
#define SCREEN_WORK_H

#include "types.h"
#include "Screen.h"
#include "Iron.h"
#include "Display.h"
#include "Buzzer.h"
#include "Encoder.h"
#include "History.h"

#define VOID_SELF void *const void_self
#define SELF ScreenWork *const self
typedef struct ScreenWorkStruct {
    Screen parent;
    Iron* pIron;                          // Pointer to the iron instance
    Display* pD;                         // Pointer to the display instance
    Buzzer* pBz;                          // Pointer to the simple Buzzer instance
    Encoder* pEnc;                        // Pointer to the rotary encoder instance
    History hTemp;
    bool heating_animation;               // Weither the heating animation is ON
    uint32_t update_screen;               // Time in ms to update the screen
    bool ready;                           // Weither the iron is ready
} ScreenWork;

void createScreenWork(SELF, Iron *iron, Display *display, Encoder *encoder, Buzzer *buzzer);

void initScreenWork(VOID_SELF);
void showScreenWork(VOID_SELF);
void rotaryValueScreenWork(VOID_SELF, int16_t value);

#undef SELF
#endif /* SCREEN_WORK_H */
