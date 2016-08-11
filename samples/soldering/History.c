#include "History.h"

void createHistory(History *const self) {
    // Not sure why there was a separate init, but keeping this as the "constructor"
    // to maintain similarity with originals use of init functions.
    self->len = 0;
}

void initHistory(History *const self) {
    self->len = 0;
}

void put(History *const self, int item) {
    if (self->len < H_LENGTH) {
        self->queue[self->len++] = item;
    } else {
        for (byte i = 0; i < self->len - 1; ++i) {
            self->queue[i] = self->queue[i+1];
        }
        self->queue[H_LENGTH - 1] = item;
    }
}

bool isFull(History *const self) {
    return (self->len == H_LENGTH);
}

int top(History *const self) {
    return self->queue[0];
}

int average(History *const self) {
    long sum = 0;
    if (self->len == 0) {
        return 0;
    }

    if (self->len == 1) {
        return self->queue[0];
    }

    for (byte i = 0; i < self->len; ++i) {
        sum += self->queue[i];
    }
    sum += self->len >> 1;                      // round the average
    sum /= self->len;

    if (sum < 0) serialWriteString("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARGH");
    return (int)sum;
}

float dispersion(History *const self) {
    if (self->len < 3) {
        return 1000;
    }

    long sum = 0;
    long avg = average(self);

    for (byte i = 0; i < self->len; ++i) {
        long q = self->queue[i];
        q -= avg;
        q *= q;
        sum += q;
    }
    sum += self->len << 1;
    float d = (float)sum / (float)self->len;
    return d;
}

float gradient(History *const self) {
    // approfimating the history with the line (y = ax+b) using method of minimum square. Gradient is parameter a
    if (self->len < 2) return 0;
    long sx, sx_sq, sxy, sy;
    sx = sx_sq = sxy = sy = 0;

    for (byte i = 1; i <= self->len; ++i) {
        sx    += i;
        sx_sq += i*i;
        sxy   += i*self->queue[i-1];
        sy    += self->queue[i-1];
    }
    long numerator   = self->len * sxy - sx * sy;
    long denominator = self->len * sx_sq - sx * sx;
    float a = (float)numerator / (float)denominator;
    return a;
}

