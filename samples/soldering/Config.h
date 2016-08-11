#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include "constants.h"
#include "arduino.h"

#define SELF Config *const self

struct cfg {
    uint16_t temp_min;                        // The minimum temperature (180 centegrees)
    uint16_t temp_max;                        // The temperature for 400 centegrees
    uint16_t temp;                            // The temperature of the iron to be start
    byte     brightness;                      // The display brightness [0-15]
    bool   celsius;                         // Temperature units: true - celsius, false - farenheit
};

typedef struct ConfigStruct {
    struct cfg Config;
    bool can_write;
    bool is_valid;
    bool save_calibration;
    byte buffRecords;
    uint16_t rAddr;
    uint16_t wAddr;
    uint16_t eLength;
    uint32_t nextRecID;
    byte record_size; // was const
} Config;

void createConfig(Config *const self);

void initConfig(Config *const self);
bool load(SELF);
bool isValid(const Config *self);
uint16_t Config_temp(const Config *self);
byte getBrightness(const Config *self);
bool Config_getTempUnits(const Config *self);
bool saveTemp(Config *const self, uint16_t t);
void saveConfig(Config *const self, byte bright, bool cels);
void saveCalibrationData(Config *const self, uint16_t t_max, uint16_t t_min);
void getCalibrationData(Config *const self, uint16_t *t_max, uint16_t *t_min);

// Private function declarations
bool readRecord(SELF, uint16_t addr, uint32_t *recID);
bool save(SELF);

#undef SELF
#endif /* CONFIG_H */
