#include "ScreenMain.h"
#include "string.h"

#define SELF ScreenMain *const self
#define PTR void *const void_self
#define CAST_SELF ScreenMain *const self = (ScreenMain *const) void_self;

const char MSG_OFF[4]  = {' ', '0', 'F', 'F'};
const char MSG_COOL[4] = {'c', '0', 'L', 'd'};
const char MSG_IDLE[4] = {'1', 'd', 'L', 'E'};

void createScreenMain(SELF,
        Iron *iron, Display *display, Encoder *encoder, Buzzer *buzzer, Config *config) {
    self->update_screen = 0;
    self->pIron = iron;
    self->pD = display;
    self->pEnc = encoder;
    self->pBz = buzzer;
    self->pCfg = config;
    self->is_celsius = true;

    memcpy(self->msg_off, MSG_OFF, 4);
    memcpy(self->msg_cool, MSG_COOL, 4);
    memcpy(self->msg_idle, MSG_IDLE, 4);

    // Override parent functions
    self->parent.init = initScreenMain;
    self->parent.showScreen = showScreenMain;
    self->parent.rotaryValue = rotaryValueScreenMain;
}


void initScreenMain(PTR) {
    CAST_SELF;

    Display *pD = self->pD;
    Iron *pIron = self->pIron;

    switchPower(pIron, false);
    uint16_t temp_set = getTemp(pIron);
    self->is_celsius = Config_getTempUnits(self->pCfg);

    setTempUnits(pIron, self->is_celsius);

    uint16_t tempH = temp2humanUnits(pIron, temp_set);
    if (self->is_celsius) {
        initEncoder(self->pEnc, tempH, temp_minC, temp_maxC, 1, 5, false); // added default arg false
    } else {
        initEncoder(self->pEnc, tempH, temp_minF, temp_maxF, 1, 5, false); // added default arg false
    }

    self->show_set_temp = false;
    self->update_screen = millis();

    clear(pD);
    forceRedraw(void_self);

    uint16_t temp = Iron_temp(pIron);
    self->used = ((temp > 400) && (temp < 740));
    self->cool_notified = !self->used;
    if (self->used) {                         // the iron was used, we should save new data in EEPROM
        saveTemp(self->pCfg, temp_set);
    }
}

void rotaryValueScreenMain(PTR, int16_t value) {
    CAST_SELF;

    Display *pD = self->pD;
    Iron *pIron = self->pIron;

    self->show_set_temp = true;
    self->update_screen = millis() + 1000;
    setTempHumanUnits(pIron, value);
    number(pD, value, 0);
    tSet(pD);
}

void showScreenMain(PTR) {
    CAST_SELF;

    Display *pD = self->pD;
    Iron *pIron = self->pIron;

    if ((!self->parent.force_redraw) && (millis() < self->update_screen)) {
        return;
    }

    self->parent.force_redraw = false;
    self->update_screen = millis();

    uint16_t temp = Iron_temp(pIron);
    if (temp > 720) {                     // No iron connected
        message(pD, self->msg_idle);
        noAnimation(pD);
        return;
    }

    if (self->show_set_temp) {
        self->update_screen += 7000;
        uint16_t temp = getTemp(pIron);
        temp = temp2humanUnits(pIron, temp);
        number(pD, temp, 0);
        tSet(pD);
    } else {
        self->update_screen += 10000;
        if (self->used && isCold(pIron)) {
            message(pD, self->msg_cool);
            if (!self->cool_notified) {
                shortBeep(self->pBz);
                self->cool_notified = true;
            }
        } else {
            message(pD, self->msg_off);
            if (self->used && !self->cool_notified) {
                cooling(pD);
            }
        }
    }
    byte hot = hotPercent(pIron);
    percent(pD, hot);
    self->show_set_temp = !self->show_set_temp;
}
