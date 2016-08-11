#ifndef SCREEN_TUNE_H
#define SCREEN_TUNE_H

#include "Screen.h"
#include "Iron.h"
#include "Display.h"
#include "Encoder.h"
#include "Buzzer.h"
#include "Config.h"
#include "History.h"
#include "oo.h"

typedef struct ScreenTuneStruct {
    Screen parent;
    Iron* pIron;                          // Pointer to the iron instance
    Display* pD;                         // Pointer to the display instance
    Encoder* pEnc;                        // Pointer to the rotary encoder instance
    Buzzer* pBz;                          // Pointer to the simple Buzzer instance
    Config* pCfg;                         // Pointer to the configuration class
    bool upper;                           // Weither tune upper or lower temperature
    bool arm_beep;                        // Weither beep is armed
    uint32_t update_screen;               // Time in ms to switch information on the display
    uint16_t tul[2];                      // upper & lower temp
    History hTemp;
    byte max_power;           // Maximum possible power to be applied
} ScreenTune;

void createScreenTune(ScreenTune *const self,
        Iron* iron, Display* display, Encoder* enc, Buzzer* buzz, Config* cfg);

void initScreenTune(VOID_SELF);
void* menuScreenTune(VOID_SELF);
void* menu_longScreenTune(VOID_SELF);
void showScreenTune(VOID_SELF);
void rotaryValueScreenTune(VOID_SELF, int16_t value);

#endif /* SCREEN_TUNE_H */
