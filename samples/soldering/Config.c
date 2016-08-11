#include "Config.h"
#include "alloc.h"
#include "eeprom.h"

#include "string.h"

#define SELF Config *const self

void createConfig(Config *const self) {
    self->can_write = FALSE;
    self->buffRecords = 0;

    self->rAddr = 0;
    self->wAddr = 0;
    self->eLength = 0;
    self->nextRecID = 0;
    self->Config.temp = 470;
    self->Config.temp_min = 417;
    self->Config.temp_max = 700;
    self->save_calibration = FALSE;

    self->record_size = 16;
}

void initConfig(Config *const self) {
    self->eLength = EEPROM_length();
    //byte t, p ,h; // changed: was unused!?!
    uint32_t recID;
    uint32_t minRecID = 0xffffffff;
    uint16_t minRecAddr = 0;
    uint32_t maxRecID = 0;
    uint16_t maxRecAddr = 0;
    byte records = 0;

    self->nextRecID = 0;

    // read all the records in the EEPROM find min and max record ID
    for (uint16_t addr = 0; addr < self->eLength; addr += self->record_size) {
        if (readRecord(self, addr, &recID)) {
            ++records;
            if (minRecID > recID) {
                minRecID = recID;
                minRecAddr = addr;
            }
            if (maxRecID < recID) {
                maxRecID = recID;
                maxRecAddr = addr;
            }
        } else {
            break;
        }
    }

    if (records == 0) {
        self->wAddr = self->rAddr = 0;
        self->can_write = true;
        return;
    }

    self->rAddr = maxRecAddr;
    if (records < (self->eLength / self->record_size)) {    // The EEPROM is not full
        self->wAddr = self->rAddr + self->record_size;
        if (self->wAddr > self->eLength) {
            self->wAddr = 0;
        }
    } else {
        self->wAddr = minRecAddr;
    }
    self->can_write = true;
}

bool isValid(const Config *self)    {
    return self->is_valid;
}

uint16_t Config_temp(const Config *self)   {
    return self->Config.temp;
}

byte getBrightness(const Config *self) {
    return self->Config.brightness;
}

bool Config_getTempUnits(const Config *self)  {
    return self->Config.celsius;
}

bool saveTemp(Config *const self, uint16_t t) {
    if (!self->save_calibration && (t == self->Config.temp)) {
        return TRUE;
    }

    self->Config.temp = t;
    self->save_calibration = FALSE;
    return save(self);
}

void saveConfig(Config *const self, byte bright, bool cels) {
    //if ((bright >= 0) && (bright <= 15)) { // Always true
    if (bright <= 15) {
        self->Config.brightness = bright;
    }
    self->Config.celsius = cels;
    save(self);
}

void saveCalibrationData(Config *const self, uint16_t t_max, uint16_t t_min) {
    self->Config.temp_max  = t_max;
    self->Config.temp_min  = t_min;
    self->save_calibration = TRUE;
}

void getCalibrationData(Config *const self, uint16_t *t_max, uint16_t *t_min) {
    *t_max = self->Config.temp_max;
    *t_min = self->Config.temp_min;
}

bool save(Config *const self) {
    if (!self->can_write) {
        return 0;
    }

    if (self->nextRecID == 0) {
        self->nextRecID = 1;
    }

    uint16_t startWrite = self->wAddr;
    uint32_t nxt = self->nextRecID;
    byte summ = 0;

    for (byte i = 0; i < 4; ++i) {
        EEPROM_write(startWrite++, nxt & 0xff);
        summ <<=2; summ += nxt;
        nxt >>= 8;
    }

    byte* p = (byte *)&self->Config;
    for (byte i = 0; i < sizeof(struct cfg); ++i) {
        summ <<= 2; summ += p[i];
        EEPROM_write(startWrite++, p[i]);
    }
    summ ++;                                  // To avoid empty records
    EEPROM_write(self->wAddr + self->record_size - 1, summ);

    self->rAddr = self->wAddr;
    self->wAddr += self->record_size;

    if (self->wAddr > EEPROM_length()) {
        self->wAddr = 0;
    }

    return true;
}

bool load(SELF) {
    self->is_valid = readRecord(self, self->rAddr, &(self->nextRecID));
    self->nextRecID++;

    if (self->is_valid) {
        if (self->Config.temp_min >= self->Config.temp_max) {
            self->Config.temp_min = 417;
            self->Config.temp_max = 700;
        }
        if ( (self->Config.temp > self->Config.temp_max)
                || (self->Config.temp < self->Config.temp_min)) {
            self->Config.temp = 470;
        }
        // if ((self->Config.brightness < 0) || (self->Config.brightness > 15)) { Always false
        if (self->Config.brightness > 15) {
            self->Config.brightness = M_INTENSITY;
        }
    }
    return self->is_valid;
}

bool readRecord(SELF, uint16_t addr, uint32_t *recID) {
    byte Buff[16];

    for (byte i = 0; i < 16; ++i) {
        Buff[i] = EEPROM_read(addr+i);
    }

    byte summ = 0;
    for (byte i = 0; i < sizeof(struct cfg) + 4; ++i) {
        summ <<= 2; summ += Buff[i];
    }
    summ ++;                                    // To avoid empty fields
    if (summ == Buff[15]) {                     // Checksumm is correct
        uint32_t ts = 0;
        for (int i = 3; i >= 0; --i) { // changed char i -> int i
            ts <<= 8;
            ts |= Buff[i];
        }
        *recID = ts;
        // byte i = 4; // changed: was unused!?!
        memcpy(&self->Config, &Buff[4], sizeof(struct cfg));
        return true;
    }
    return false;
}
