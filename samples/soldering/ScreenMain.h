#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include "Screen.h"
#include "Iron.h"
#include "Display.h"
#include "Encoder.h"
#include "Buzzer.h"
#include "Config.h"

#define SELF ScreenMain *const self
typedef struct ScreenMainStruct {
    Screen parent;

    Iron *pIron;                        // Pointer to the iron instance
    Display *pD;                       // Pointer to the display instance
    Encoder *pEnc;                      // Pointer to the rotary encoder instance
    Buzzer *pBz;                       // Pointer to the simple buzzer instance
    Config *pCfg;                      // Pointer to the configuration instance

    bool show_set_temp;                 // Weither show the temperature was set or 'OFF' string
    uint32_t update_screen;             // Time in ms to switch information on the display
    bool used;                          // Weither the iron was used (was hot)
    bool cool_notified;                 // Weither there was cold notification played
    bool is_celsius;                    // The temperature units (Celsius or farenheit)
    char msg_off[4];
    char msg_cool[4];
    char msg_idle[4];
} ScreenMain;

void createScreenMain(SELF,
        Iron *iron, Display *display, Encoder *encoder, Buzzer *buzzer, Config *config);

void initScreenMain(void *const ptr);
void showScreenMain(void *const ptr);
void rotaryValueScreenMain(void *const ptr, int16_t value);


#undef SELF
#endif /* SCREEN_MAIN_H */
