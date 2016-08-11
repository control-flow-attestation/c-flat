#include "Encoder.h"

#include "arduino.h"

const uint16_t fast_timeout = 300;        // Time in ms to change encodeq quickly
const uint16_t overPress = 1000;

void createEncoderD(Encoder *const self, byte aPIN, byte bPIN) {
    createEncoder(self, aPIN, bPIN, 0);
}

void createEncoder(Encoder *const self, byte aPIN, byte bPIN, int16_t initPos) {
    self->pt = 0;
    self->mPIN = aPIN;
    self->sPIN = bPIN;
    self->pos = initPos;

    self->min_pos = -32767;
    self->max_pos = 32766;
    self->channelB = FALSE;
    self->increment = 1;
    self->changed = 0;
    self->is_looped = FALSE;

    self->fast_timeout = 300;        // Time in ms to change encodeq quickly
    self->overPress = 1000;
}

void initEncoderD(Encoder *const self, int16_t initPos, int16_t low, int16_t upp) {
    initEncoder(self, initPos, low, upp, 1, 0, false);
}

void initEncoder(Encoder *const self, int16_t initPos, int16_t low, int16_t upp, byte inc, byte fast_inc, boolean looped) {
    self->min_pos = low;
    self->max_pos = upp;

    if (!write(self, initPos)) {
        initPos = self->min_pos;
    }

    self->fast_increment = inc;
    self->increment = inc;

    if (fast_inc > self->increment) {
        self->fast_increment = fast_inc;
    }

    self->is_looped = looped;
    pinMode(self->mPIN, INPUT_PULLUP);
    pinMode(self->sPIN, INPUT_PULLUP);
}

void reset(Encoder *const self, int16_t initPos, int16_t low, int16_t upp, byte inc) {
    self->min_pos = low;
    self->max_pos = upp;

    if (!write(self, initPos)) {
        // this does seem to do anything but change an unused local variable?!?
        initPos = self->min_pos;
    }

    self->increment = inc;
}

void set_increment(Encoder *const self, byte inc) {
    self->increment = inc;
}

byte get_increment(Encoder *const self) {
    return self->increment;
}

boolean write(Encoder *const self, int16_t initPos) {
    if ((initPos >= self->min_pos) && (initPos <= self->max_pos)) {
        self->pos = initPos;
        return TRUE;
    }
    return FALSE;
}

int16_t read(Encoder *const self) {
    return self->pos;
}

void changeEncoderINTR(Encoder *const self) {
    // Interrupt function, called when the channel A of encoder changed

    boolean rUp = digitalRead(self->mPIN);
    unsigned long now_t = millis();

    if (!rUp) {                                 // The channel A has been "pressed"
        if ((self->pt == 0) || (now_t - self->pt > self->overPress)) {
            self->pt = now_t;
            self->channelB = digitalRead(self->sPIN);
        }
    } else {
        if (self->pt > 0) {
            byte inc = self->increment;

            if ((now_t - self->pt) < self->overPress) {
                if ((now_t - self->changed) < self->fast_timeout) {
                    inc = self->fast_increment;
                }

                self->changed = now_t;
                if (self->channelB) {
                    self->pos -= inc;
                } else {
                    self->pos += inc;
                }

                if (self->pos > self->max_pos) {
                    if (self->is_looped) {
                        self->pos = self->min_pos;
                    } else {
                        self->pos = self->max_pos;
                    }
                }
                if (self->pos < self->min_pos) {
                    if (self->is_looped) {
                        self->pos = self->max_pos;
                    } else {
                        self->pos = self->min_pos;
                    }
                }
            }
            self->pt = 0;
        }
    }
}
