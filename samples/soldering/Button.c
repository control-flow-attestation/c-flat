#include "Button.h"
#include "arduino.h"
#include "alloc.h"

const uint16_t tickTimeout = 200;          // Period of button tick, while tha button is pressed
const uint16_t shortPress = 900;           // If the button was pressed less that this timeout, we assume the short button press

void createButtonD(Button *const self, byte buttonPIN) {
    createButton(self, buttonPIN, 3000);
}

void createButton(Button *const self, byte buttonPIN, unsigned int timeout_ms) {
    self->pt = 0;
    self->tickTime = 0;
    self->buttonPIN = buttonPIN;
    self->overPress = timeout_ms;
}

void initButton(Button *const self) {
    pinMode(self->buttonPIN, INPUT_PULLUP);
}

void setTimeoutD(Button *const self) {
    setTimeout(self, 3000);
}

void setTimeout(Button *const self, uint16_t timeout_ms) {
    self->overPress = timeout_ms;
}

byte intButtonStatus(Button *const self) {
    byte m = self->mode;
    self->mode = 0;
    return m;
}

void changeButtonINTR(Button *const self) {
    // Interrupt function, called when the button status changed

    boolean keyUp = digitalRead(self->buttonPIN);
    unsigned long now_t = millis();
    if (!keyUp) {                                // The button has been pressed
        if ((self->pt == 0) || (now_t - self->pt > self->overPress)) {
            self->pt = now_t;
        }
    } else {
        if (self->pt > 0) {
            if ((now_t - self->pt) < shortPress) {
                self->mode = 1; // short press
            } else {
                self->mode = 2;                         // long press
            }
            self->pt = 0;
        }
    }
}


byte buttonCheck(Button *const self) {
    // Check the button state, called each time in the main loop

    byte mode = 0;
    boolean keyUp = digitalRead(self->buttonPIN);         // Read the current state of the button
    uint32_t now_t = millis();
    if (!keyUp) {                                // The button is pressed
        if ((self->pt == 0) || (now_t - self->pt > self->overPress)) {
            self->pt = now_t;
        }
    } else {
        if (self->pt == 0) {
            return 0;
        }

        if ((now_t - self->pt) > shortPress) {
            // Long press
            self->mode = 2;
        } else {
            self->mode = 1;
        }
        self->pt = 0;
    }
    return mode;
}

boolean buttonTick(Button *const self) {
    // When the button pressed for a while, generate periodical ticks

    // Read the current state of the button
    boolean keyUp = digitalRead(self->buttonPIN);
    uint32_t now_t = millis();

    if (!keyUp && (now_t - self->pt > shortPress)) {   // The button have been pressed for a while
        if (now_t - self->tickTime > tickTimeout) {
            self->tickTime = now_t;
            return (self->pt != 0);
        }
    } else {
        if (self->pt == 0) {
            return FALSE;
        }
        self->tickTime = 0;
    }
    return FALSE;
}
