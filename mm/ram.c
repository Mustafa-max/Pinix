/*
 * vmpinix Memory Management Subsystem - ram.c
 * Physical RAM Allocator and Page Frame Management Implementation
 */

#include "memory.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define MAX_PHYSICAL_PAGES 262144 // 1GB RAM (4KB Pages)

typedef struct {
    unsigned char bitmap[MAX_PHYSICAL_PAGES / 8];
    unsigned long total_pages;
    unsigned long free_pages;
} physical_ram_manager_t;

static physical_ram_manager_t ram_mgr;

int ram_subsystem_init(void) {
    ram_mgr.total_pages = MAX_PHYSICAL_PAGES;
    ram_mgr.free_pages = MAX_PHYSICAL_PAGES;

    for (unsigned long i = 0; i < (MAX_PHYSICAL_PAGES / 8); i++) {
        ram_mgr.bitmap[i] = 0; // Tüm sayfalar başlangıçta boş
    }

    return 0;
}

void* ram_allocate_frame(void) {
    for (unsigned long i = 0; i < ram_mgr.total_pages; i++) {
        unsigned long byte_idx = i / 8;
        unsigned int bit_idx = i % 8;

        if (!(ram_mgr.bitmap[byte_idx] & (1 << bit_idx))) {
            ram_mgr.bitmap[byte_idx] |= (1 << bit_idx);
            ram_mgr.free_pages--;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return (void*)0; // RAM Dolu
}

void ram_free_frame(void *physical_addr) {
    unsigned long frame_idx = (unsigned long)physical_addr / PAGE_SIZE;
    if (frame_idx >= ram_mgr.total_pages) return;

    unsigned long byte_idx = frame_idx / 8;
    unsigned int bit_idx = frame_idx % 8;

    if (ram_mgr.bitmap[byte_idx] & (1 << bit_idx)) {
        ram_mgr.bitmap[byte_idx] &= ~(1 << bit_idx);
        ram_mgr.free_pages++;
    }
}

unsigned long long ram_get_free_bytes(void) {
    return ram_mgr.free_pages * PAGE_SIZE;
}