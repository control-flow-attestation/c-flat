#ifndef LED_CONTROL_H
#define LED_CONTROL_H
/*
 * API docs taken from http://playground.arduino.cc/Main/LedControl#NumberSeg7Control
 */

#define LED_CHARS_LENGTH 4
#define LED_IS_VALID_DIGIT(x) (x <= LED_CHARS_LENGTH && x >=  0)

#include "types.h"

typedef struct LecControlStruct {
    char chars[LED_CHARS_LENGTH];
} LedControl;

void createLedControl(LedControl *const self, int DIN, int CLK, int CS);

/**
 * Send updated view view serial
 */
void emulate_showLed(LedControl *const self);

void shutdown(LedControl *const self, int a, boolean b);
void setIntensity(LedControl *const self, int a, int b);

/* Switch all Leds on the display off.
 * Params:
 *   addr The address of the display to control
 *
 * void clearDisplay(int *addr);
 */
void clearDisplay(LedControl *const self);

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
void setDigit(LedControl *const self, int addr, int digit, byte value, boolean dp);

/* Display a character on a 7-Segment display.
 * Params:
 *   addr  address of the display
 *   digit the position of the character on the display (0..7)
 *   value the character to be displayed.
 *   dp    sets the decimal point.
 */
void setChar(LedControl *const self, int addr, int digit, char value, boolean dp);

/* Set all 8 Led's in a row to a new state
 * Params:
 *   addr   ddress of the display
 *   row    row which is to be set (0..7)
 *   value  each bit set to 1 will light up the corresponding Led.
 */
void setRow(LedControl *const self, int addr, int row, byte value);

#endif /* LED_CONTROL_H */
