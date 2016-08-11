#ifndef IRON_H
#define IRON_H

#include "types.h"
#include "History.h"

#define SELF Iron *const self
typedef struct IronStruct {
    History h_power;
    uint32_t checkMS;                     // Milliseconds to measure the temperature next time
    byte hPIN, sPIN;                      // the heater PIN and the sensor PIN
    int  power;                           // The soldering station power
    bool on;                              // Weither the soldering iron is on
    bool fix_power;                       // Weither the soldering iron is set the fix power
    bool unit_celsius;                    // Human readable units for the temparature (celsius or farenheit)
    int  temp_set;                        // The temperature that should be established
    int  temp_hist[2];                    // previously measured temperature
    bool iron_checked;                    // Weither the iron works
    int  temp_start;                      // The temperature when the solder was switched on
    int  elapsed_time;                    // The time elipsed from the start (in seconds)
    uint16_t temp_min;                    // The minimum temperature (180 centegrees)
    uint16_t temp_max;                    // The maximum temperature (400 centegrees)
    bool pid_iterate;                     // Weither the inerative PID formulae can be used
    uint16_t temp_cold;      // The cold temperature to touch the iron safely
    byte max_power;           // maximum power to the iron (180)
    byte min_power;            // minimum power to the iron
    byte delta_t;               // measurement error of the temperature
    uint16_t period;          // The period to check the soldering iron temperature, ms
    int check_time;            // Time in seconds to check weither the solder is heating
    int heat_expected;         // The iron should change the temperature at check_time
    byte denominator_p;        // The common coefficeient denominator power of 2 (10 means 1024)
    int Kp;                  // Kp multiplied by denominator
    int Ki;                   // Ki multiplied by denominator
    int Kd;                   // Kd multiplied by denominator
} Iron;

void createIron(SELF, byte heater_pin, byte sensor_pin);

void initIron(SELF, uint16_t t_max, uint16_t t_min);

bool isOn(SELF);
void switchPower(SELF, bool On);
bool isCold(SELF);
void setTempUnits(SELF, bool celsius);
bool getTempUnits(SELF);
uint16_t getTemp(SELF);

/**
 * Set the temperature to be keeped
 */
void setTemp(SELF, int t);

/**
 * Set the temperature to be keeped in human readable units (celsius or farenheit)
 */
void setTempHumanUnits(SELF, int t);

/**
 * Translate internal temperature to the celsius or farenheit
 */
uint16_t temp2humanUnits(SELF, uint16_t temp);

/**
 * The actual temperature of the soldering iron
 */
uint16_t Iron_temp(SELF);

uint16_t lastTemp(SELF);

byte getPower(SELF);                  // power that is applied to the soldering iron [0 - 255]
byte getAvgPower(SELF);               // average applied power
byte appliedPower(SELF);              // Power applied to the solder [0-10]
byte hotPercent(SELF);                // How hot is the iron (used in the idle state)
void keepTemp(SELF);                  // Main solder iron loop
bool fixPower(SELF, byte Power);            // Set the specified power to the the soldering iron

// private: -> Iron.c
//void applyPower(SELF);                // Check the the power limits and apply power to the heater
//void resetPID(SELF);                  // Reset PID algorythm parameters

#undef SELF
#endif /* IRON_H */
