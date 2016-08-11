#include "ScreenPower.h"
#include "arduino.h"

#define SELF ScreenPower *const self

void createScreenPower(SELF, Iron* iron, Display* display, Encoder *encoder) {
    self->pIron = iron;
    self->pD = display;
    self->pEnc = encoder;
    self->on = false;

    self->max_power = 100;

    // overrides
    self->parent.init = initScreenPower;
    self->parent.showScreen = showScreenPower;
    self->parent.rotaryValue = rotaryValueScreenPower;
    //FIXME: fails for some reason
    //self->parent.menu = menuScreenPower;
    //self->parent.menu_long = menu_longScreenPower;
}

void initScreenPower(VOID_SELF) {
    CAST_SELF(ScreenPower);
    Iron *pIron = self->pIron;

    byte p = getAvgPower(pIron);
    initEncoderD(self->pEnc, p, 0, self->max_power); // defaults:  false
    // Do start heating immediately
    self->on = true;
    switchPower(pIron, false);
    fixPower(pIron, p);
    initHistory(&self->hTemp);
    clear(self->pD);
}

void showScreenPower(VOID_SELF) {
    CAST_SELF(ScreenPower);
    Iron *pIron = self->pIron;
    Display *pD = self->pD;

    if ((!self->parent.force_redraw) && (millis() < self->update_screen)) return;

    self->parent.force_redraw = false;

    uint16_t temp = Iron_temp(pIron);
    put(&self->hTemp, temp);
    int avg = average(&self->hTemp);
    temp = temp2humanUnits(pIron, avg);
    number(pD, temp, 0);
    noAnimation(pD);
    byte p = appliedPower(pIron);
    percent(pD, p);
    self->update_screen = millis() + 500;

}

void rotaryValueScreenPower(VOID_SELF, int16_t value) {
    CAST_SELF(ScreenPower);
    Display *pD = self->pD;

    number(pD, value, 0);
    P(pD);
    byte p = appliedPower(self->pIron);
    percent(pD, p);
    if (self->on) {
        fixPower(self->pIron, value);
    }
    self->update_screen = millis() + 1000;
}

void* menuScreenPower(VOID_SELF) {
    CAST_SELF(ScreenPower);

    self->on = !self->on;

    if (self->on) {
        uint16_t pos = read(self->pEnc);
        self->on = fixPower(self->pIron, pos);
    } else {
        fixPower(self->pIron, 0);
    }
    return self;
}

void* menu_longScreenPower(VOID_SELF) {
    CAST_SELF(ScreenPower);

    self->on = !self->on;

    if (self->on) {
        uint16_t pos = read(self->pEnc);
        self->on = fixPower(self->pIron, pos);
    } else {
        fixPower(self->pIron, 0);
    }
    return self;
}

