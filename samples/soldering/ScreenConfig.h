#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#include "Screen.h"
#include "Display.h"
#include "Iron.h"
#include "Encoder.h"
#include "Config.h"
#include "oo.h"

typedef struct ScreenConfigStruct {
    Screen parent;
    Iron* pIron;                          // Pointer to the iron instance
    Display* pD;                         // Pointer to the display instance
    Encoder* pEnc;                        // Pointer to the rotary encoder instance
    Config*  pCfg;                        // Pointer to the config instance
    uint32_t update_screen;               // Time in ms to update the screen
    byte mode;                            // Which parameter to change: 0 - brightness, 1 - C/F, 2 - tuneSCREEN
    bool tune;                            // Weither the parameter is modified
    bool changed;                         // Weither some configuration parameter has been changed
    byte brigh;                           // Current screen brightness
    bool cels;                            // Current celsius/farenheit;
    char msg_celsius[4];
    char msg_farenheit[4];
} ScreenConfig;

void createScreenConfig(ScreenConfig *const self, Iron* iron, Display* display, Encoder* enc, Config* cfg);
void initScreenConfig(VOID_SELF);
void showScreenConfig(VOID_SELF);
void rotaryValueScreenConfig(VOID_SELF, int16_t value);
void* menuScreenConfig(VOID_SELF);
void* menu_longScreenConfig(VOID_SELF);

#endif /* SCREEN_CONFIG_H */
