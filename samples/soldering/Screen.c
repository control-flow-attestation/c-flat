#include "Screen.h"

#include "arduino.h"
#include "serial.h"

#define SELF Screen *const self

void initScreen(SELF) {
    if (self->init != 0) {
        self->init(self);
    }
}

void showScreen(SELF) {
    if (self->showScreen != 0) {
        self->showScreen(self);
    }
}

void* menu(SELF) {
    if (self->menu != 0) {
        return self->menu(self);
    }

    if (self->next != 0) {
        return self->next;
    } else {
        return self;
    }
}
void* menu_long(SELF) {
    if (self->menu_long != 0) {
        return self->menu_long(self);
    }

    if (self->nextL != 0) {
        return self->nextL;
    } else {
        return self;
    }
}

void rotaryValue(SELF, int16_t value) {
    if (self->rotaryValue != 0) {
        self->rotaryValue(self, value);
        return;
    }
}

bool isSetup(SELF) {
    return (self->scr_timeout != 0);
}

void forceRedraw(SELF) {
    self->force_redraw = true;
}

void *returnToMain(SELF) {
    if (self->main && (self->scr_timeout != 0) && (millis() >= self->time_to_return)) {
        self->scr_timeout = 0;
        return self->main;
    }
    return self;
}

void resetTimeout(SELF) {
    if (self->scr_timeout > 0) {
        self->time_to_return = millis() + (self->scr_timeout)*1000;
    }
}

void setSCRtimeout(SELF, uint16_t t) {
    self->scr_timeout = t;
    resetTimeout(self);
}



