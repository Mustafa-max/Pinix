/*
 * vmpinix Standard Library - stdlib.h
 * General Utilities, Conversion, and Memory Management Wrappers
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include "stdint.h"

static inline int atoi(const char *str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    while (str[i] != '\0' && str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    return res * sign;
}

static inline void abort(void) {
    __asm__ volatile ("cli; hlt");
    for (;;);
}

#endif