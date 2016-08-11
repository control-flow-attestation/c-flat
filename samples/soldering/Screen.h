#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

#define SELF Screen *const self
typedef struct ScreenStruct {
    void *next;                       // Pointer to the next screen
    void *nextL;                      // Pointer to the next Level screen, usually, setup
    void *main;                       // Pointer to the main screen
    bool force_redraw;
    uint16_t scr_timeout;               // Timeout is sec. to return to the main screen, canceling all changes
    uint32_t time_to_return;            // Time in ms to return to main screen
    void (*init)(void *const ptr);
    void (*showScreen)(void *const ptr);
    void *(*menu)(void *const ptr);
    void *(*menu_long)(void *const ptr);
    void (*rotaryValue)(void *const ptr, int16_t value);
    bool (*isSetup)(void *const ptr);
    void (*forceRedraw)(void *const ptr);
    void (*retrunToMain)(void *const ptr);
    void (*resetTimeout)(void *const ptr);
    void (*setSCRtimeout)(void *const ptr, uint16_t t);
} Screen;

void initScreen(SELF);
void showScreen(SELF);
void *menu(SELF);
void *menu_long(SELF);
void rotaryValue(SELF, int16_t value);
bool isSetup(SELF);
void forceRedraw(SELF);
void *returnToMain(SELF);
void resetTimeout(SELF);
void setSCRtimeout(SELF, uint16_t t);

#undef SELF
#endif /* SCREEN_H */

