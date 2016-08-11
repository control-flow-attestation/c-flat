#include "Buzzer.h"
#include "arduino.h"
#include "alloc.h"

void createBuzzer(Buzzer *const self, byte pin) {
    self->buzzerPIN = pin;
}

void shortBeep(Buzzer *const self)  {
    tone(self->buzzerPIN , 3520, 80);
}
