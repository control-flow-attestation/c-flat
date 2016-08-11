/**
 * Some, what seems to me to be, Arduino specifics API functions
 *
 * Constnats as needed from here (filled with bogus values):
 * https://www.arduino.cc/en/Reference/Constants
 *
 */
#ifndef ARDUINO_H
#define ARDUINO_H

#include "types.h"
#include "LedControl.h"
#include "Encoder.h"
#include "Iron.h"

// Digital pin modes
#define INPUT_PULLUP (byte)1
#define INPUT (byte)2
#define OUTPUT (byte)3

// Interrupt modes, see https://www.arduino.cc/en/Reference/AttachInterrupt
#define CHANGE 1

// Pin Levels (at least soldering implementation relies on LOW == 0
#define LOW (byte)0
#define HIGH (byte)1

// Pin Mappings
// https://www.arduino.cc/en/Tutorial/AnalogInputPins
#define A0 (byte)67


/**
 * This needs to be run once per progrma loop to advance time
 */
void emulate_tick(void);
bool emulate_State(LedControl *const disp, byte buttonMode, Encoder *const enc, Iron *iron);

unsigned long millis(void);
void delay(int ms);

int abs(int value);

void tone(byte buzzerPIN, int frequency, int duraion);
void pinMode(byte pin, byte mode);
byte digitalRead(byte pin);


/** https://www.arduino.cc/en/Reference/DigitalWrite
 *
 * Parameters:
 *
 * pin: the pin number
 *
 * value: HIGH or LOW
 *
 * Returns: void
 */
void digitalWrite(byte pin, byte value);

/** https://www.arduino.cc/en/Reference/AnalogRead
 *
 * Parameters:
 *
 * pin: the number of the analog input pin to read from (0 to 5 on most boards, 0 to 7 on the Mini
 * and Nano, 0 to 15 on the Mega)
 *
 * Returns: int (0 to 1023)
 */
int analogRead(byte pin);


/** https://www.arduino.cc/en/Reference/AnalogWrite
 *
 * Parameters:
 *
 * pin: the pin to write to.
 * value: the duty cycle: between 0 (always off) and 255 (always on).
 *
 * Returns: nothing
 */
void analogWrite(byte pin, byte value);

/** https://www.arduino.cc/en/Reference/Map
 *
 * Re-maps a number from one range to another. That is, a value of fromLow would get mapped to
 * toLow, a value of fromHigh to toHigh, values in-between to values in-between, etc.
 */
int map(int value, int fromLow, int fromHigh, int toLow, int toHigh);

/** https://www.arduino.cc/en/Reference/AttachInterrupt
 *
 * attachInterrupt(digitalPinToInterrupt(pin), ISR, mode);
 */
void attachInterrupt(byte pin, void (*ISR)(void), byte mode);

byte digitalPinToInterrupt(byte pin);


#endif /* ARDUINO_H */
