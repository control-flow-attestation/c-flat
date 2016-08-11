#ifndef BUTTON_H
#define BUTTON_H

#include "types.h"

typedef struct ButtonStruct {
    volatile byte mode;                        // The button mode: 0 - not pressed, 1 - pressed, 2 - long pressed
    uint16_t overPress;                        // Maxumum time in ms the button can be pressed
    volatile uint32_t pt;                      // Time in ms when the button was pressed (press time)
    uint32_t tickTime;                         // The time in ms when the button Tick was set
    byte buttonPIN;                            // The pin number connected to the button
} Button;

void createButtonD(Button *const self, byte buttonPIN); //with default: 3000
void createButton(Button *const self, byte buttonPIN, unsigned int timeout_ms);

void initButton(Button *const self);

void setTimeoutD(Button *const self); // uint16_t timeout_ms = 3000
void setTimeout(Button *const self, uint16_t timeout_ms);

byte intButtonStatus(Button *const self);

void changeButtonINTR(Button *const self);
byte buttonCheck(Button *const self);
boolean buttonTick(Button *const self);

#endif /* BUTTON_H */

