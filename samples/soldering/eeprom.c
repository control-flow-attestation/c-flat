#include "eeprom.h"

// This proably wouldn't need to be as much?
#define MEM_SIZE 100
byte EEPROM_memory[MEM_SIZE];

uint16_t EEPROM_length(void) {
    return MEM_SIZE;
}

void EEPROM_write(const byte addr, const byte data) {
    EEPROM_memory[addr] = data;
}

byte EEPROM_read(const byte addr) {
    return EEPROM_memory[addr];
}
