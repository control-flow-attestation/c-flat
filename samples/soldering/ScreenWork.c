#include "ScreenWork.h"
#include "constants.h"
#include "Screen.h"

#include "serial.h"

#define SELF ScreenWork *const self
#define CAST_SELF ScreenWork *const self = (ScreenWork *const) void_self

void createScreenWork(SELF, Iron *iron, Display *display, Encoder *encoder, Buzzer *buzzer) {
    self->update_screen = 0;
    self->pIron = iron;
    self->pD    = display;
    self->pBz   = buzzer;
    self->pEnc   = encoder;
    self->heating_animation = false;
    self->ready = false;

    // Overrides
    self->parent.rotaryValue = rotaryValueScreenWork;
    self->parent.showScreen = showScreenWork;
    self->parent.init = initScreenWork;
}


void initScreenWork(VOID_SELF) {
    CAST_SELF;

    Iron *pIron = self->pIron;
    Encoder *pEnc = self->pEnc;

    uint16_t temp_set = getTemp(pIron);
    bool is_celsius = getTempUnits(pIron);
    uint16_t tempH = temp2humanUnits(pIron, temp_set);

    if (is_celsius) {
        initEncoder(pEnc, tempH, temp_minC, temp_maxC, 1, 5, false); // Added default false
    }
    else {
        initEncoder(pEnc, tempH, temp_minF, temp_maxF, 1, 5, false); // Added default false
    }
    switchPower(pIron, true);
    self->heating_animation = false;
    self->ready = false;

    initHistory(&self->hTemp);
    clear(self->pD);
    forceRedraw(void_self);
}

void rotaryValueScreenWork(VOID_SELF, int16_t value) {
    CAST_SELF;

    self->heating_animation = false;
    self->ready = false;
    self->update_screen = millis() + 2000;
    setTempHumanUnits(self->pIron, value);
    number(self->pD, value, 0);
    tSet(self->pD);
}

void showScreenWork(VOID_SELF) {
    CAST_SELF;
    Iron *pIron = self->pIron;
    Display *pD = self->pD;

    if ((!self->parent.force_redraw) && (millis() < self->update_screen)) {
        return;
    }

    self->parent.force_redraw = false;
    self->update_screen = millis() + 1000;

    uint16_t temp = lastTemp(pIron);
    uint16_t temp_set = getTemp(pIron);
    put(&self->hTemp, temp);
    int avg = average(&self->hTemp);

    temp = temp2humanUnits(pIron, avg);
    number(pD, temp, 0);
    byte p = appliedPower(pIron);
    percent(pD, p);

    if ((abs(temp_set - avg) < 4) && (dispersion(&self->hTemp) < 15))  {
        noAnimation(pD);
        self->heating_animation = false;
        if (!self->ready) {
            shortBeep(self->pBz);
            self->ready = true;
        }
        return;
    }
    if (!self->ready && temp < temp_set) {
        if (!self->heating_animation) {
            self->heating_animation = true;
            heating(pD);
        }
    }
}
