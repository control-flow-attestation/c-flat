#include "Iron.h"
#include "arduino.h"
#include "constants.h"

#define SELF Iron *const self

// private:
void applyPower(SELF);                // Check the the power limits and apply power to the heater
void resetPID(SELF);                  // Reset PID algorythm parameters

void createIron(SELF, byte heater_pin, byte sensor_pin) {
    // create* child objects
    createHistory(&self->h_power);

    // "const" values
    self->temp_cold  = 255;      // The cold temperature to touch the iron safely
    self->max_power = 220;           // maximum power to the iron (180)
    self->min_power = 25;            // minimum power to the iron
    self->delta_t = 2;               // measurement error of the temperature
    self->period = 500;          // The period to check the soldering iron temperature, ms
    self->check_time = 10;            // Time in seconds to check weither the solder is heating
    self->heat_expected = 10;         // The iron should change the temperature at check_time
    self->denominator_p = 10;        // The common coefficeient denominator power of 2 (10 means 1024)
    self->Kp = 5120;                  // Kp multiplied by denominator
    self->Ki = 512;                   // Ki multiplied by denominator
    self->Kd = 768;                   // Kd multiplied by denominator

    // Constructor
    self->hPIN = heater_pin;
    self->sPIN = sensor_pin;
    self->on = false;
    self->unit_celsius = true;
    self->fix_power = false;
    self->unit_celsius = true;
}

void initIron(SELF, uint16_t t_max, uint16_t t_min) {
    pinMode(self->sPIN, INPUT);
    pinMode(self->hPIN, OUTPUT);
    digitalWrite(self->hPIN, LOW);
    self->on = false;
    self->fix_power = false;
    self->power = 0;
    self->checkMS = 0;

    self->elapsed_time = 0;
    self->temp_start = analogRead(self->sPIN);
    self->iron_checked = false;
    self->temp_max = t_max;
    self->temp_min = t_min;

    resetPID(self);
    initHistory(&self->h_power);
}

bool isOn(SELF) {
    return self->on;
}

void switchPower(SELF, bool On) {
    self->on = On;
    if (!self->on) {
        digitalWrite(self->hPIN, LOW);
        self->fix_power = false;
        return;
    }

    resetPID(self);
    self->temp_hist[1] = analogRead(self->sPIN);
    initHistory(&self->h_power);
    self->checkMS = millis();
}

bool isCold(SELF) {
    return (Iron_temp(self) < self->temp_cold);
}

void setTempUnits(SELF, bool celsius) {
    self->unit_celsius = celsius;
}

bool getTempUnits(SELF) {
    return self->unit_celsius;
}

uint16_t getTemp(SELF) {
    return self->temp_set;
}

uint16_t lastTemp(SELF) {
    return self->temp_hist[1];
}

/**
 * Set the temperature to be keeped
 */
void setTemp(SELF, int t) {
    if (self->on) {
        resetPID(self);
    }
    self->temp_set = t;
}

/**
 * Set the temperature to be keeped in human readable units (celsius or farenheit)
 */
void setTempHumanUnits(SELF, int t) {
    int temp;
    if (self->unit_celsius) {
        if (t < temp_minC) {
            t = temp_minC;
        }
        if (t > temp_maxC) {
            t = temp_maxC;
        }
        temp = map(t+1, temp_minC, temp_maxC, self->temp_min, self->temp_max);
    } else {
        if (t < temp_minF) {
            t = temp_minF;
        }
        if (t > temp_maxF) {
            t = temp_maxF;
        }
        temp = map(t+2, temp_minF, temp_maxF, self->temp_min, self->temp_max);
    }
    for (byte i = 0; i < 10; ++i) {
        int tH = temp2humanUnits(self, temp);
        if (tH <= t) {
            break;
        }
        --temp;
    }
    setTemp(self, temp);
}

/**
 * Translate internal temperature to the celsius or farenheit
 */
uint16_t temp2humanUnits(SELF, uint16_t temp) {
    if (!self->unit_celsius) {
        return map(temp, self->temp_min, self->temp_max, temp_minF, temp_maxF);
    }
    return map(temp, self->temp_min, self->temp_max, temp_minC, temp_maxC);
}

void resetPID(SELF) {
    self->pid_iterate = false;
    self->temp_hist[0] = self->temp_hist[1] = 0;
}

uint16_t Iron_temp(SELF) {
    uint16_t t1 = analogRead(self->sPIN);
    delay(50);
    uint16_t t2 = analogRead(self->sPIN);

    if (abs(t1 - t2) < 50) {
        t1 += t2 + 1;
        // average of two measurements
        t1 >>= 1;
        return t1;
    }
    if (abs(t1 - self->temp_hist[1]) < abs(t2 - self->temp_hist[1])) {
        return t1;
    } else {
        return t2;
    }
}

uint16_t lastTemp(SELF);

byte getPower(SELF) {
    int p = self->power;
    if (p < 0) p = 0;
    if (p > self->max_power) {
        p = self->max_power;
    }
    return p & 0xff;
}

byte getAvgPower(SELF) {
    int p = average(&self->h_power);
    return p & 0xff;
}

byte appliedPower(SELF) {
    byte p = getPower(self);
    return map(p, 0, self->max_power, 0, 10);
}

byte hotPercent(SELF) {
    uint16_t t = Iron_temp(self);
    byte r = map(t, self->temp_cold, self->temp_set, 0, 10);
    // if (r < 0) r = 0; // always false
    return r;
}

void keepTemp(SELF) {
    if (self->checkMS > millis()) {
        return;
    }

    self->checkMS = millis() + self->period;

    if (!self->on) {
        // If the soldering iron is set to be switched off
        if (!self->fix_power) {
            // Surely power off the iron
            digitalWrite(self->hPIN, LOW);
        }
        return;
    }

    int temp_curr = Iron_temp(self);

    // Check weither the iron can be heated
    if (!self->iron_checked) {
        self->elapsed_time += self->period / 1000;

        if (self->elapsed_time >= self->check_time) {
            if ((abs(self->temp_set - temp_curr) < 100)
                    || ((temp_curr - self->temp_start) > self->heat_expected)) {
                self->iron_checked = true;
            } else {
                // Prevent the iron damage
                switchPower(self, false);
                self->elapsed_time = 0;
                self->temp_start = analogRead(self->sPIN);
                self->iron_checked = false;
            }
        }
    }

    // first, use the direct formulae, not the iterate process
    if (self->temp_hist[0] == 0) {
        long p = (long) self->Kp * (self->temp_set - temp_curr) +
            (long) self->Ki * (self->temp_set - temp_curr);

        p += (1 << (self->denominator_p - 1));
        p >>= self->denominator_p;

        self->temp_hist[1] = temp_curr;

        // If the temperature is near, prepare the PID iteration process
        if ((self->temp_set - temp_curr) < 30) {
            // The second loop
            if (self->pid_iterate) {
                // Now we are redy to use iterate algorythm
                self->temp_hist[0] = self->temp_hist[1];
            } else {
                // The first loop
                self->pid_iterate = true;
            }
        }
        self->power = p;
    } else {
        long delta_p = (long) self->Kp * (self->temp_hist[1] - temp_curr);
        delta_p += (long)self->Ki * (self->temp_set - temp_curr);
        delta_p += (long)self->Kd * (2*self->temp_hist[1] - self->temp_hist[0] - temp_curr);
        delta_p += (1 << (self->denominator_p-1));
        delta_p >>= self->denominator_p;
        self->power += delta_p;
        self->temp_hist[0] = self->temp_hist[1];
        self->temp_hist[1] = temp_curr;
    }
    applyPower(self);
}

void applyPower(SELF) {
    byte p = getPower(self);
    if (self->temp_hist[1] > (self->temp_set + 1)) {
        p = 0;
    }
    if (p == 0) {
        digitalWrite(self->hPIN, LOW);
    }
    if (self->on) {
        analogWrite(self->hPIN, p & 0xff);
    }
    put(&self->h_power, p);
}

bool fixPower(SELF, byte Power) {
    if (Power == 0) {                     // To switch off the iron, set the power to 0
        self->fix_power = false;
        digitalWrite(self->hPIN, LOW);
        return true;
    }

    if (Power > 80) {
        return false;
    }

    if (!self->fix_power) {
        self->fix_power = true;
        self->power = Power;
        analogWrite(self->hPIN, self->power & 0xff);
    } else {
        if (self->power != Power) {
            self->power = Power;
            analogWrite(self->hPIN, self->power & 0xff);
        }
    }
    return true;
}
