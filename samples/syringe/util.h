/*
 * Utility API
 */

#define INT_MAX 2147483647

void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);
int analogRead(int pin);

void Serial_begin(int baud);
int Serial_available();
int Serial_read();
int Serial_write(char* output, int len);

unsigned long millis();
void delayMicroseconds(float usecs);

int toUInt(char* input, int len);
