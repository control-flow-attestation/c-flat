#include "serial.h"
#include "rpi/aux.h"

void serialWrite(char *const output, const int len) {
    for (int i = 0; i < len; i++) {
        rpi_aux_mini_uart_write(output[i]);
    }
}

void serialWriteString(char *const output) {
    int i = 0;
    while(1) {
        if (output[i] == '\0') {
            return;
        }
        rpi_aux_mini_uart_write(output[i++]);

        if (i > 100) {
            serialWriteString("String too long, stopping output");
            return;
        }
    }
}

void serialWriteInt(int value) {
    // FIXME: make sure this actually works
    int i = 0;
    char str[20];

    if (value == 0) {
        char c = '0';
        serialWrite(&c, 1);
        return;
    }

    for (; value > 0; i++) {
        str[i] = '0' + (value%10);
        value = value / 10;
    }
    // serialWriteString("<num>");
    i--;
    for (; i >= 0; i--) {
        serialWrite(&str[i], 1);
    }
    // serialWriteString("</num>");
}

void serialWriteNewLine(void) {
    rpi_aux_mini_uart_write('\n');
    rpi_aux_mini_uart_write('\r');
}

int serialRead(void) {
    return rpi_aux_mini_uart_read();
}

bool serialHasData(void) {
    return rpi_aux_mini_uart_data_available();
}


