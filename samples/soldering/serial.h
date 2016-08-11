#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

void serialWrite(char *const output, const int len);
/**
 * NOTE: must be '\0' terminated!
 */
void serialWriteString(char *const output);
void serialWriteNewLine(void);
void serialWriteInt(int value);
bool serialHasData(void);
int serialRead(void);


#endif /* SERIAL_H */

