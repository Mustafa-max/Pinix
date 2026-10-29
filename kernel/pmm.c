/*
 * vmpinix Core Kernel - pmm.c
 * Physical Memory Manager and Page Frame Allocator Bridge
 */

#include "mm.h"

// mm/ram.c içerisindeki fiziksel ram fonksiyonlarına köprü
extern void* ram_allocate_frame(void);
extern void ram_free_frame(void *physical_addr);

int pmm_init(void) {
    // Fiziksel bellek yöneticisini doğrula ve başlat
    return 0;
}

void* pmm_alloc_page(void) {
    return ram_allocate_frame();
}

void pmm_free_page(void *page_address) {
    ram_free_frame(page_address);
}