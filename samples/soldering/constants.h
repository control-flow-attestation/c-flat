#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "types.h"
#include "arduino.h"

// max7219 interface
#define M_DIN 12
#define M_CLK 10
#define M_CS  11
// display intensity (overwitten by brhghtness configuration parameter)
#define M_INTENSITY 3

// Rotary Encoder main pin (right)
#define R_MAIN_PIN 2
// Rotary Encoder second pin (left)
#define R_SECD_PIN 4
#define R_BUTN_PIN 3
// Rotary Encoder push button pin

// Thermometer pin from soldering iron
#define PROBEPIN A0
// soldering iron heater pin
#define HEATERPIN 5
// simple buzzer to make a noise
#define BUZZERPIN 6

#define TEMP_MIN 180
#define TEMP_MAX 400

// Minimum temperature in degrees of celsius
#define temp_minC TEMP_MIN
// Maximum temperature in degrees of celsius
#define temp_maxC TEMP_MAX
#define temp_minF (TEMP_MIN * 9 + 32*5 + 2)/5
#define temp_maxF (TEMP_MAX * 9 + 32*5 + 2)/5

#endif /* CONSTANTS_H */
