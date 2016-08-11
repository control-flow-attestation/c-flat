#ifndef SCEEN_ERROR_H
#define SCEEN_ERROR_H

#include "Display.h"
#include "Screen.h"

typedef struct ScreenErrorStruct {
    Screen parent;
    Display *pD;
    char msg_fail[4];
} ScreenError;

void createScreenError(ScreenError *const self, Display *display);
void initScreenError(void *const void_self);

#endif /* SCEEN_ERROR_H */

