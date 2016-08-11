#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"
#include "LedControl.h"

typedef struct DisplayStruct {
    LedControl parent;
    byte intensity;                           // The display brightness
    byte dot_mask;                            // the decimal dot mask
    boolean big_number;                          // If the number is big, we cannot use highest digit for animation
    byte animate_type;                        // animation type: 0 - off, 1 - heating, 2 - cooling
    byte animate_count;                       // The number of the bytes in the animation
    byte animate_index;                       // Current byte in the animation
    uint32_t animate_ms;                      // The time in ms when animation should change to the symbol
    // the following we're const:
    uint16_t animate_speed;           // milliseconds to switch next byte in the animation string (100)
    byte a_heating[5];
    byte a_cooling[4];
    byte setup_mode[3][4];
} Display;


void createDisplay(Display *const self, byte DIN, byte CLK, byte CS, byte BRIGHT);
void initDisplay(Display *const self);
void clear(Display *const self);

/**
 * Set the display brightness
 */
void brightness(Display *const self, byte BRIGHT);

/**
 * Set the digital number to be displayed
 */
void number(Display *const self, int data, byte dot_mask);

/**
 * Set the 4-shar word to be displayed
 */
void message(Display *const self, char msg[4]);

/**
 * The highest digit animate heating process
 */
void heating(Display *const self);

// The highest digit animate cooling process
void cooling(Display *const self);

void P(Display *const self);                             // The highest digit show 'setting power' process
void tSet(Display *const self);                          // THe highest digit show 'temperature set' information (idle state)
void upper(Display *const self);                         // The highest digit show 'setting upper temperature' process
void lower(Display *const self);                         // The highest digit show 'setting lower temperature' process
void setupMode(Display *const self, byte mode);                // Show the onfigureation mode [0 - 2]
void noAnimation(Display *const self);                   // Switch off the animation in the highest digit
void Display_show(Display *const self);                          // Show the data on the 4-digit indicator
void percent(Display *const self, byte Power);                 // Show the percentage on the led bar (for example power supplied)

#endif /* DISPLAY_H */
