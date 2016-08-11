#include "ScreenError.h"

#include "string.h" // memcpy
#include "oo.h"

#define CAST_SELF(type) type *const self = (type *const) void_self

const char MSG_FAIL[4] = {'F', 'A', '1', 'L'};

void createScreenError(ScreenError *const self, Display *display) {
    self->pD =  display;
    memcpy(self->msg_fail, MSG_FAIL, 4);

    // Override methods
    self->parent.init = initScreenError;
}

void initScreenError(void *const void_self) {
    CAST_SELF(ScreenError);

    clear(self->pD);
    message(self->pD, self->msg_fail);
}
