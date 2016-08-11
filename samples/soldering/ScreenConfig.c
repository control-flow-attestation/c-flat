#include "ScreenConfig.h"
#include "string.h"

const char MSG_CELSIUS[4]   = {' ', ' ', ' ', 'c' };
const char MSG_FARENHEIT[4] = {' ', ' ', ' ', 'F' };

void createScreenConfig(ScreenConfig *const self, Iron* iron, Display* display, Encoder* enc, Config* cfg) {
    self->pIron = iron;
    self->pD = display;
    self->pEnc = enc;
    self->pCfg = cfg;

    memcpy(self->msg_celsius, MSG_CELSIUS, 4);
    memcpy(self->msg_farenheit, MSG_FARENHEIT, 4);

    // overrides
    self->parent.init = initScreenConfig;
    self->parent.showScreen = showScreenConfig;
    self->parent.rotaryValue = rotaryValueScreenConfig;
    self->parent.menu = menuScreenConfig;
    self->parent.menu_long = menu_longScreenConfig;
}

void initScreenConfig(VOID_SELF) {
    CAST_SELF(ScreenConfig);

    self->mode = 0;
    initEncoder(self->pEnc, self->mode, 0, 2, 1, 0, true);
    self->tune    = false;
    self->changed = false;
    self->brigh   = getBrightness(self->pCfg);
    self->cels    = Config_getTempUnits(self->pCfg);
    clear(self->pD);
    setSCRtimeout(&self->parent, 30);
}

void showScreenConfig(VOID_SELF) {
    CAST_SELF(ScreenConfig);
    Display *pD = self->pD;

    if ((!self->parent.force_redraw) && (millis() < self->update_screen)) {
        return;
    }

    self->parent.force_redraw = false;
    self->update_screen = millis() + 10000;

    if ((self->mode == 1) && self->tune) {
        if (self->cels) {
            message(pD, self->msg_celsius);
        } else {
            message(pD, self->msg_farenheit);
        }
        return;
    }
    setupMode(pD, self->mode);
}

void rotaryValueScreenConfig(VOID_SELF, int16_t value) {
    CAST_SELF(ScreenConfig);
    Display *pD = self->pD;

    self->update_screen = millis() + 10000;
    if (self->tune) {
        self->changed = true;
        if (self->mode == 0) {
            self->brigh = value;
            brightness(pD, self->brigh);
        } else {                              // mode == 1, C/F
            self->cels = !value;
        }
    } else {
        self->mode = value;
    }
    self->parent.force_redraw = true;
}

void* menuScreenConfig(VOID_SELF) {
    CAST_SELF(ScreenConfig);
    Encoder *pEnc = self->pEnc;

    if (self->tune) {
        self->tune = false;
        initEncoder(pEnc, self->mode, 0, 2, 1, 0, true);
    } else {
        switch (self->mode) {
            case 0:                             // brightness
                initEncoder(pEnc, self->brigh, 0, 15, 1, 0, false);
                break;
            case 1:                             // C/F
                initEncoder(pEnc, self->cels, 0, 1, 1, 0, true );
                break;
            case 2:                             // Calibration
                if (self->parent.next) {
                    return self->parent.next;
                }
                break;
        }
        self->tune = true;
    }
    self->parent.force_redraw = true;
    return self;
}

void* menu_longScreenConfig(VOID_SELF) {
    CAST_SELF(ScreenConfig);

    if (self->parent.nextL) {
        if (self->changed) {
            saveConfig(self->pCfg, self->brigh, self->cels);
            setTempUnits(self->pIron, self->cels);
        }
        return self->parent.nextL;
    }
    return (Screen*)self;
}

