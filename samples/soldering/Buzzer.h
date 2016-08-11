#ifndef BUZZER_H
#define BUZZER_H

#include "types.h"

typedef struct BuzzerStruct {
    byte buzzerPIN;
} Buzzer;

void createBuzzer(Buzzer *const self, byte pin);
void shortBeep(Buzzer *const self);

#endif /* BUZZER_H */
