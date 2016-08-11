#ifndef UTIL_H
#define UTIL_H

#include "serial.h"
#include "types.h"

#define UNIMPLEMENTED(x) {\
    serialWriteString("Unimplmemented: "); \
    serialWriteString(x); \
    serialWriteNewLine(); \
}

char hexDigitToChar(const byte input);
int toUInt(char* input, int len);

#endif /* UTIL_H */
