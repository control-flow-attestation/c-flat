#include "Display.h"
#include "alloc.h"
#include "string.h"
#include "arduino.h"

const int HEATING_SIZE = 5;
const byte HEATING[5] = {
    '^', '^', '^', '^', '^'
    //0b00001000, 0b00010100, 0b00000001, 0b00100010, 0b01000000
};

const int COOLING_SIZE = 4;
const byte COOLING[4] = {
    '_', '_', '_', '_'
    //0b01000000, 0b00000001, 0b00001000, 0
};

const int SETUP_MODE_SIZE = 12;
const byte SETUP_MODE[3][4] = {
    {'b','r','g','t'},
    {' ','C','-','F'},
    {'t','u','n','e'},
    //{0b00011111, 0b00000101, 0b01111011, 0b00001111},   // 'brgt'
    //{0b00000000, 0b01001110, 0b00000001, 0b01000111},   // 'C-F'
    //{0b00001111, 0b00011100, 0b00010101, 0b01001111}    // 'tune'
};

void createDisplay(Display *const self, byte DIN, byte CLK, byte CS, byte BRIGHT) {
    createLedControl(&self->parent, (int)DIN, (int)CLK, (int)CS);
    self->intensity = BRIGHT;

    self->animate_speed = 100;

    memcpy(self->a_heating, HEATING, HEATING_SIZE);
    memcpy(self->a_cooling, COOLING, COOLING_SIZE);
    memcpy(self->setup_mode, SETUP_MODE, SETUP_MODE_SIZE);
}

void initDisplay(Display *const self) {
    shutdown(&self->parent, 0, FALSE);
    setIntensity(&self->parent, 0, self->intensity);
    clear(self);

    delay(500);

    self->dot_mask = 0;
    self->big_number = FALSE;
    noAnimation(self);
}

void clear(Display *const self) {
    clearDisplay(&self->parent);
}

void brightness(Display *const self, byte BRIGHT) {
    if (BRIGHT > 15) {
        BRIGHT = 15;
    }
    self->intensity = BRIGHT;
    setIntensity(&self->parent, 0, self->intensity);
}

void noAnimation(Display *const self) {;
    // Switch off the animation in the highest digit
    self->animate_type = 0;
    self->animate_count = 0;
    self->animate_index = 0;
    self->animate_ms = 0;
}

void number(Display *const self, int data, byte a_dot_mask) {
    self->big_number = (data >= 1000);
    byte i;
    byte m = 1;

    for (i = 0; i < 4; ++i) {
        if (data == 0 && i != 0) { break; }

        byte s = data % 10;
        data /= 10;

        setDigit(&self->parent, 0, 3-i, s, (m & a_dot_mask));
        m <<= 1;
    }

    for (; i < 4; ++i) {
        setChar(&self->parent, 0, 3-i, ' ', (m & a_dot_mask));
        m <<= 1;
    }
}

void message(Display *const self, char msg[4]) {
    for (byte i = 0; i < 4; ++i) {
        setChar(&self->parent, 0, i, msg[i], FALSE);
    }
    self->animate_type = 0;
}

void setupMode(Display *const self, byte mode) {
    // Show the onfigureation mode [0 - 2]
    if (mode <= 2) { // mode >= 0 always true
        for (byte i = 0; i < 4; ++i) {
            setRow(&self->parent, 0, i, self->setup_mode[mode][i]);
        }
    }
}

void heating(Display *const self) {
    if (self->big_number) {
        return;
    }
    self->animate_type = 1;
    self->animate_count = 5;
    self->animate_index = 0;
    self->animate_ms = 0;
    self->animate_ms = millis() + self->animate_speed;
    setRow(&self->parent, 0, 0, self->a_heating[0]);
}

void cooling(Display *const self) {
    if (self->big_number) {
        return;
    }
    self->animate_type = 2;
    self->animate_count = 4;
    self->animate_index = 0;
    self->animate_ms = 0; // pointless?!?
    self->animate_ms = millis() + self->animate_speed * 5;
    setRow(&self->parent, 0, 0, self->a_cooling[0]);
}

void P(Display *const self) {
    // The highest digit show 'setting power' process
    if (self->big_number) {
        return;
    }
    self->animate_type = 0;
    setChar(&self->parent, 0, 0, 'P', TRUE);
}

void upper(Display *const self) {
    // The highest digit show 'setting upper temperature' process
    if (self->big_number) {
        return;
    }
    self->animate_type = 0;
    //setRow(&self->parent, 0, 0, 0b01100011);
    setRow(&self->parent, 0, 0, 'u');
}

void lower(Display *const self) {
    // The highest digit show 'setting lower temperature' process
    if (self->big_number) {
        return;
    }
    self->animate_type = 0;
    //setRow(&self->parent, 0, 0, 0b00011101);
    setRow(&self->parent, 0, 0, 'l');
}

void tSet(Display *const self) {
    // THe highest digit show 'temperature set' information (idle state)
    if (self->big_number) {
        return;
    }
    self->animate_type = 0;
    //setRow(&self->parent, 0, 0, 0b10001111);
    setRow(&self->parent, 0, 0, 's');
}

void Display_show(Display *const self) {
    // Show the data on the 4-digit indicator
    if (!self->big_number && self->animate_type) {
        if ((self->animate_count > 1) && (self->animate_ms < millis())) {
            if(++(self->animate_index) >= self->animate_count) {
                self->animate_index = 0;
            }

            switch (self->animate_type) {
                case 1:
                    self->animate_ms = millis() + self->animate_speed;
                    setRow(&self->parent, 0, 0, self->a_heating[self->animate_index]);
                    break;
                case 2:
                    self->animate_ms = millis() + self->animate_speed * 5;
                    setRow(&self->parent, 0, 0, self->a_cooling[self->animate_index]);
                    break;
                default:
                    break;
            }
        }
    }
}

void percent(Display *const self, byte Power) {
    // Show the percentage on the led bar (for example power supplied)
    if (Power > 10) {
        Power = 10;
    }
    uint16_t mask = 0;

    for (byte i = 0; i < Power; ++i) {
        mask >>= 1;
        mask |= 0x8000;                           // 15-th bit
    }

    byte m1 = mask >> 8;
    byte m2 = mask & 0xff;

    setRow(&self->parent, 0, 4, m1);
    setRow(&self->parent, 0, 5, m2);
}
