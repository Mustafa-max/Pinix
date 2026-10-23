/*
 * vmpinix Memory Management Subsystem - lib.h
 * Internal Memory Utility Macros and Helper Definitions
 */

#ifndef _MM_LIB_H
#define _MM_LIB_H

#include "memory.h"

static inline unsigned long align_to_page(unsigned long address) {
    if ((address & (PAGE_SIZE - 1)) == 0) {
        return address;
    }
    return (address & PAGE_MASK) + PAGE_SIZE;
}

static inline int is_page_aligned(unsigned long address) {
    return (address & (PAGE_SIZE - 1)) == 0;
}

#endif