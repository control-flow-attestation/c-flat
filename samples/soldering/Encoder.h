#ifndef ENCODER_H
#define ENCODER_H

#include "types.h"

typedef struct EncoderStruct {
    int32_t min_pos;
    int32_t max_pos;
    volatile uint32_t pt;                     // Time in ms when the encoder was rotaded
    volatile uint32_t changed;                // Time in ms when the value was changed
    volatile boolean channelB;
    volatile int32_t pos;                     // Encoder current position
    byte mPIN;
    byte sPIN;                          // The pin numbers connected to the main channel and to the socondary channel
    boolean is_looped;                           // Weither the encoder is looped
    byte increment;                           // The value to add or substract for each encoder tick
    byte fast_increment;                      // The value to change encoder when in runs quickly
    /*const*/ uint16_t fast_timeout;        // Time in ms to change encodeq quickly
    /*const*/ uint16_t overPress;
} Encoder;

void createEncoderD(Encoder *const self, byte aPIN, byte bPIN); // default initPos = 0
void createEncoder(Encoder *const self, byte aPIN, byte bPIN, int16_t initPos);

void initEncoderD(Encoder *const self, int16_t initPos, int16_t low, int16_t upp); // defalt values: byte inc = 1, byte fast_inc = 0, bool looped = false
void initEncoder(Encoder *const self, int16_t initPos, int16_t low, int16_t upp, byte inc, byte fast_inc, boolean looped);

void reset(Encoder *const self, int16_t initPos, int16_t low, int16_t upp, byte inc);

void set_increment(Encoder *const self, byte inc);
byte get_increment(Encoder *const self);
boolean write(Encoder *const self, int16_t initPos);
int16_t read(Encoder *const self);
void changeEncoderINTR(Encoder *const self);

#endif /* ENCODER_H */



