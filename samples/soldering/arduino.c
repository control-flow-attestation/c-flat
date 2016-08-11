#include "arduino.h"
#include "util.h"
#include "serial.h"
#include "constants.h"

#include <time.h>

// This is just a guess, and is probably highly inaccurate for timing (used at least in delay)
//
// The soldering samples sources indicate RPi timing functions would help, if these become available
// one coudle use those for uptime and delay and remove updateTime and related globals.
//
#define LOOPS_PER_MS 150
#define MS_PER_TICK 1

// NORMAL_TEMPERATURE should probably be kept above Iron.temp_min (which seems to be 417?)
#define NORMAL_TEMPERATURE 256
#define MAX_TEMP 700
#define ANALOG_MAX 1023

long uptime = 0;

byte R_BUTN_PIN_value = LOW;
byte R_MAIN_PIN_value = LOW;
byte R_SECD_PIN_value = LOW;

void (*R_BUTN_PIN_callback)(void);
void (*R_MAIN_PIN_callback)(void);

int PROBEPIN_value = NORMAL_TEMPERATURE;
int PROBEPIN_old = NORMAL_TEMPERATURE;
int HEATERPIN_value = LOW;

int tempCounter = 0;

int16_t oldRotVal = 0;

char stateDisplay[4] = { ' ', ' ', ' ', ' ' };
byte stateButtonMode = 0;

void handleInput(void);
void updateTime(void);
void updateTemperature(void);

byte R_MAIN_PIN_old = LOW;
byte R_SECD_PIN_old = LOW;

bool emulate_State(LedControl *const disp, byte buttonMode, Encoder *const enc, Iron *iron) {
    bool changed = false;

    for (int i = 0; i < 4; i++) {
        char d = disp->chars[i];
        if (stateDisplay[i] != d) {
            stateDisplay[i] = d;
            changed = true;
        }
    }

    if (stateButtonMode != buttonMode) {
        stateButtonMode = buttonMode;
        changed = true;
    }

    if (R_MAIN_PIN_value != R_MAIN_PIN_old) {
        changed = true;
        R_MAIN_PIN_old = R_MAIN_PIN_value;
    }

    if (R_SECD_PIN_value != R_SECD_PIN_old) {
        changed = true;
        R_SECD_PIN_old = R_SECD_PIN_value;
    }

    if (oldRotVal != read(enc)) {
        oldRotVal = read(enc);
        changed = true;
    }

    if (PROBEPIN_old != PROBEPIN_value) {
        PROBEPIN_old = PROBEPIN_value;
        changed = true;
    }


    if (changed) {
        emulate_showLed(disp);

        serialWriteString(" (");

        serialWriteString("btn mode: ");
        char c = '0' + buttonMode;
        serialWrite(&c, 1);

        serialWriteString(", rot: ");
        serialWriteInt(enc->pos);
        serialWriteString(" [");
        serialWriteInt(enc->min_pos);
        serialWriteString(", ");
        serialWriteInt(enc->max_pos);
        serialWriteString("]");

        serialWriteString(", iron: ");
        serialWriteInt(PROBEPIN_value);
        serialWriteString(" ->-> ");
        serialWriteInt(iron->temp_set);

        serialWriteString(")");
    }

    return changed;
}



void emulate_tick() {
    updateTime();
    updateTemperature();
    handleInput();
}

void updateTemperature() {
    if (tempCounter >= 400) {
        tempCounter = 0;
        if (HEATERPIN_value && PROBEPIN_value < ANALOG_MAX) {
            int target = map(HEATERPIN_value, 0, 255, NORMAL_TEMPERATURE, MAX_TEMP);
            int diff = target - PROBEPIN_value;
            PROBEPIN_value += (diff < 0 ? -1 : (diff < 10 ? 1 : diff / 10));

            /*
            serialWriteString("HEATERPIN_value is ");
            serialWriteInt(HEATERPIN_value);
            serialWriteNewLine();
            serialWriteString("Target is ");
            serialWriteInt(target);
            serialWriteNewLine();
            serialWriteString("PROBEPIN_value is ");
            serialWriteInt(PROBEPIN_value);
            serialWriteNewLine();
            */
        } else if (PROBEPIN_value > NORMAL_TEMPERATURE) {
            PROBEPIN_value -= 2;
            if (PROBEPIN_value < NORMAL_TEMPERATURE) PROBEPIN_value = NORMAL_TEMPERATURE;
        }
    }
    tempCounter++;
}

long previousTimeSend = 0;
void updateTime(void) {
    uptime += MS_PER_TICK;
    /*
       if (uptime - previousTimeSend > 1000) {
       serialWriteString("one second\r\n");
       previousTimeSend = uptime;
       }
       */
}


void handleInput(void) {
    while (serialHasData()) {
        int data = serialRead();
        char c = (char) data;
        switch(c) {
            case('j'):
            case('k'):
                {
                    byte newValue = (c == 'j' ? LOW : HIGH);
                    if (R_BUTN_PIN_value != newValue) {
                        // notify callback if changed
                        R_BUTN_PIN_callback();
                    }
                    R_BUTN_PIN_value = newValue; // button pressed => LOW
                    break;
                }
            case('h'):
            case('l'):
                {
                    if (R_MAIN_PIN_value == HIGH) {
                        byte value = (c == 'h' ? LOW  : HIGH);
                        R_MAIN_PIN_value = LOW;
                        R_SECD_PIN_value = value;
                    } else {
                        R_MAIN_PIN_value = HIGH;
                    }
                    R_MAIN_PIN_callback();
                }
                break;
            default:
                serialWriteString("Got unrecognized key: ");
                serialWrite(&c, 1);
                serialWriteNewLine();
        }
    }
}


unsigned long millis(void) {
    return uptime;
}

void delay(int ms) {
    while(1) {
        for(int i = 0; i < LOOPS_PER_MS; i++) { /* Busy wait */ }
        ms--;
        if (ms <= 0) {
            return;
        }
    }
    uptime += ms;
}

int abs(int value) {
    if (value >= 0) {
        return value;
    }
    return -1 * value;
}

void tone(byte buzzerPIN, int frequency, int duraion) {
    UNIMPLEMENTED("arduino : tone");
    // TODO
    (void) buzzerPIN;
    (void) frequency;
    (void) duraion;
}


void pinMode(byte pin, byte mode) {
    //UNIMPLEMENTED("arduino : pinMode");
    (void) pin;
    (void) mode;
}

byte digitalRead(byte pin) {
    switch(pin) {
        case R_BUTN_PIN:
            return R_BUTN_PIN_value;
        case R_MAIN_PIN:
            return R_MAIN_PIN_value;
        case R_SECD_PIN:
            return R_SECD_PIN_value;
        default:
            serialWriteString("digitalRead, unimplemented pin: ");
            char c = '0' + pin;
            serialWrite(&c, 1);
            serialWriteNewLine();
            return LOW;
    }
}

void digitalWrite(byte pin, byte value) {
    switch(pin) {
        case HEATERPIN:
            HEATERPIN_value = value;
            break;
        default:
            serialWriteString("digitalWrite, unimplemented pin: ");
            char c = '0' + pin;
            serialWrite(&c, 1);
            serialWriteNewLine();
    }
}

int analogRead(byte pin) {
    switch(pin) {
        case HEATERPIN:
            return HEATERPIN_value;
            break;
        case PROBEPIN:
            return PROBEPIN_value;
            break;
        default:
            serialWriteString("analogRead, unimplemented pin: ");
            char c = '0' + pin;
            serialWrite(&c, 1);
            serialWriteNewLine();
            return 0;
    }
}

void analogWrite(byte pin, byte value) {
    switch(pin) {
        case HEATERPIN:
            HEATERPIN_value = value;
            break;
        default:
            serialWriteString("analogWrite, unimplemented pin: ");
            char c = '0' + pin;
            serialWrite(&c, 1);
            serialWriteNewLine();
    }
}

int map(int value, int fromLow, int fromHigh, int toLow, int toHigh) {
    /*
    if (value < fromLow || value > fromHigh) {
        // DEBUG
        serialWriteString("Mapping value not within range");
        serialWriteInt(value);
        serialWriteString(" -> [");
        serialWriteInt(fromLow);
        serialWriteString(", ");
        serialWriteInt(fromHigh);
        serialWriteString("]\r\n");
    }
    */
    int fromRange = fromHigh - fromLow;
    int fromDiff = fromHigh - value;
    float diff = (float)fromRange / (float)fromDiff;
    int toRange = toHigh - toLow;
    return toHigh - (toRange / diff);
}

void attachInterrupt(byte pin, void (*ISR)(void), byte mode) {
    (void) mode;
    switch(pin) {
        case R_BUTN_PIN:
            R_BUTN_PIN_callback = ISR;
            break;
        case R_MAIN_PIN:
            R_MAIN_PIN_callback = ISR;
            break;
        default:
            serialWriteString("digitalRead, unimplemented pin: ");
            char c = '0' + pin;
            serialWrite(&c, 1);
            serialWriteNewLine();
    }
}

byte digitalPinToInterrupt(byte pin) {
    // This is used only twice in solderingIron.c
    //attachInterrupt(digitalPinToInterrupt(R_MAIN_PIN), rotEncChange,   CHANGE);
    //attachInterrupt(digitalPinToInterrupt(R_BUTN_PIN), rotPushChange,  CHANGE);
    return pin;
}
