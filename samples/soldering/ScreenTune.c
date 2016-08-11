#include "ScreenTune.h"
#include "oo.h"

// "private functions"
void saveTempParam(ScreenTune *const self);

void createScreenTune(ScreenTune *const self,
        Iron* iron, Display* display, Encoder* enc, Buzzer* buzz, Config* cfg) {
    self->update_screen = 0;
    self->pIron = iron;
    self->pD = display;
    self->pEnc = enc;
    self->pBz  = buzz;
    self->pCfg = cfg;
    self->upper = true;

    self->max_power = 100;

    // overrides
    self->parent.init = initScreenTune;
    self->parent.showScreen = showScreenTune;
    self->parent.rotaryValue = rotaryValueScreenTune;
    self->parent.menu = menuScreenTune;
    self->parent.menu_long = menu_longScreenTune;
}

void saveTempParam(ScreenTune *const self) {
    uint16_t pos = read(self->pEnc);
    (void) pos; // unused (also in original)?
    uint16_t temp = Iron_temp(self->pIron);
    put(&self->hTemp, temp);
    temp = average(&self->hTemp);
    byte indx = 1;
    if (self->upper) {
        indx = 0;
    }
    self->tul[indx] = temp;
}

void initScreenTune(VOID_SELF) {
    CAST_SELF(ScreenTune);

    byte p = 75;
    initEncoder(self->pEnc, p, 0, self->max_power, 1, 0, false); //defaults 0, false
    fixPower(self->pIron, p);
    self->update_screen = millis();
    self->upper = true;
    self->arm_beep = true;
    self->tul[0] = self->tul[1] = 0;
    initHistory(&self->hTemp);
    forceRedraw((Screen*) self);
}

void* menuScreenTune(VOID_SELF) {
    CAST_SELF(ScreenTune);
    Display *pD = self->pD;

    saveTempParam(self);

    uint16_t temp = average(&self->hTemp);
    number(pD, temp, 0);
    if (self->upper) {
        upper(pD);
    } else {
        lower(pD);
    }
    self->update_screen = millis() + 1000;
    self->upper = !self->upper;
    initHistory(&self->hTemp);
    self->arm_beep = true;
    self->parent.force_redraw = true;
    return self;
}

void* menu_longScreenTune(VOID_SELF) {
    CAST_SELF(ScreenTune);
    Iron *pIron = self->pIron;

    saveTempParam(self);
    fixPower(pIron, 0);
    bool all_data = true;

    for (byte i = 0; i < 2; ++i) {
        if (!self->tul[i]) {
            all_data = false;
        }
    }
    if (all_data) {                         // save calibration data
        saveCalibrationData(self->pCfg, self->tul[0], self->tul[1]);
    }

    if (self->parent.nextL) {
        return self->parent.nextL;
    }
    return self;
}

void rotaryValueScreenTune(VOID_SELF, int16_t value) {
    CAST_SELF(ScreenTune);
    Display *pD = self->pD;
    Iron *pIron = self->pIron;

    number(pD, value, 0);
    if (self->upper) {
        upper(pD);
    } else {
        lower(pD);
    }
    fixPower(pIron, value);
    self->update_screen = millis() + 1000;
}

void showScreenTune(VOID_SELF) {
    CAST_SELF(ScreenTune);
    Display *pD = self->pD;
    Iron *pIron = self->pIron;

    if ((!self->parent.force_redraw) && (millis() < self->update_screen)) {
        return;
    }

    self->parent.force_redraw = false;
    self->update_screen = millis() + 1000;

    int16_t temp = Iron_temp(pIron);
    put(&self->hTemp, temp);
    temp = average(&self->hTemp);
    number(pD, temp, 0);
    if (self->upper) {
        upper(pD);
    } else {
        lower(pD);
    }
    byte p = appliedPower(pIron);
    percent(pD, p);
    if (self->arm_beep && (dispersion(&self->hTemp) < 15)) {
        shortBeep(self->pBz);
        self->arm_beep = false;
    }
}
