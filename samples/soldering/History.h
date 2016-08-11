#ifndef HISTORY_H
#define HISTORY_H

#include "types.h"

#define H_LENGTH 8

typedef struct HistoryStruct {
    byte len;
    int queue[H_LENGTH];
} History;

void createHistory(History *const self);

void initHistory(History *const self);
void put(History *const self, int item);
boolean isfull(History *const self);
int top(History *const self);
int average(History *const self);
float dispersion(History *const self);
float gradient(History *const self);

#endif /* HISTORY_H */
