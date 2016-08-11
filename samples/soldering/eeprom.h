#ifndef EEPROM_H
#define EEPROM_H

#include "types.h"

uint16_t EEPROM_length(void);
void EEPROM_write(const byte addr, const byte data);
byte EEPROM_read(const byte addr);

#endif /* EEPROM_H */
