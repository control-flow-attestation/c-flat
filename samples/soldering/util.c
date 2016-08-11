#include "util.h"

char hexDigitToChar(const byte input) {
    return input + '0';
}

/**
 * "Stolen" from the syringe sample.
 */
int toUInt(char* input, int len) {
    unsigned int output = 0;
    unsigned int overflow = 0;
    int i;

    for(i=0; i < len; i++)
    {
        if(0x30 <= input[i] && input[i] <= 0x39)
            break;
    }

    for( ; i < len; i++){
        if(input[i] < 0x30 || 0x39 < input[i])
            break;
        overflow = output;
        output = (output * 10) + (int)(input[i]) - 0x30;
        if(output < overflow){ /* report overflow */
            return INT_MAX;
        }
    }

    return output;
}

