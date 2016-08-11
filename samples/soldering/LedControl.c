#include "LedControl.h"
#include "serial.h"
#include "util.h"

#define SELF LedContorl *const self

void createLedControl(LedControl *const self, int DIN, int CLK, int CS) {
    (void) DIN;
    (void) CLK;
    (void) CS;
    clearDisplay(self);
}

void shutdown(LedControl *const self, int a, boolean b) {
    UNIMPLEMENTED("LedControl : shutdown");
    (void) self;
    (void) a;
    (void) b;
}

void setIntensity(LedControl *const self, int a, int b) {
    UNIMPLEMENTED("LedControl : setIntentisty");
    (void) self;
    (void) a;
    (void) b;
}

/* Switch all Leds on the display off.
 * Params:
 *   addr The address of the display to control
 *
 * void clearDisplay(int *addr);
 */
void clearDisplay(LedControl *const self) {
    for (int i = 0; i < LED_CHARS_LENGTH; i++) {
        self->chars[i] = ' ';
    }
}

/*
 *
 * Display a (hexadecimal) digit on a 7-Segment Display
 * Params:
 * addr  address of the display
 * digit the position of the digit on the display (0..7)
 * value the value to be displayed. (0x00..0x0F)
 * dp    sets the decimal point.
 *
 * void setDigit(int addr, int digit, byte value, boolean dp)
 */
void setDigit(LedControl *const self, int addr, int digit, byte value, boolean dp) {
    (void) addr;
    (void) dp;

    self->chars[digit] = hexDigitToChar(value);
}

/* Display a character on a 7-Segment display.
 * Params:
 *   addr  address of the display
 *   digit the position of the character on the display (0..7)
 *   value the character to be displayed.
 *   dp    sets the decimal point.
 */
void setChar(LedControl *const self, int addr, int digit, char value, boolean dp) {
    (void) addr;
    (void) digit;
    (void) dp;

    self->chars[digit] = value;
}

/* Set all 8 Led's in a row to a new state
 * Params:
 *   addr   ddress of the display
 *   row    row which is to be set (0..7)
 *   value  each bit set to 1 will light up the corresponding Led.
 */
void setRow(LedControl *const self, int addr, int row, byte value) {
    (void) addr;
    self->chars[row] = (char) value;
}


void emulate_showLed(LedControl *const self) {
    serialWriteString("D>>> ");
    serialWrite(self->chars, 4);
    serialWriteString(" <<<D");
}
