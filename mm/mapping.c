/*
 * vmpinix Memory Management Subsystem - mapping.c
 * Virtual Memory Mapping, Page Table Entry (PTE) and Directory Management
 */

#include "memory.h"

// pde_t ve pte_t tanımları projede eksikse derleme hatasını önlemek için:
#ifndef _PTE_PDE_DEFINED
#define _PTE_PDE_DEFINED
typedef unsigned int pde_t;
typedef unsigned int pte_t;
#endif

#ifndef PAGE_MASK
#define PAGE_MASK 0xFFFFF000
#endif

extern void* ram_allocate_frame(void);

int mapping_map_page(pde_t *page_directory, unsigned long virtual_addr, unsigned long physical_addr, unsigned int flags) {
    unsigned long pd_index = (virtual_addr >> 22) & 0x3FF;
    unsigned long pt_index = (virtual_addr >> 12) & 0x3FF;

    pde_t *pd = page_directory;
    pte_t *pt;

    if (!(pd[pd_index] & 1)) {
        // Sayfa tablosu henüz yok, yeni fiziksel çerçeve tahsis et
        void *new_pt = ram_allocate_frame();
        if (!new_pt) return -1;

        pd[pd_index] = ((unsigned long long)new_pt) | flags | 1;
        pt = (pte_t*)new_pt;
        for (int i = 0; i < 1024; i++) pt[i] = 0;
    } else {
        pt = (pte_t*)((pd[pd_index] & PAGE_MASK));
    }

    pt[pt_index] = (physical_addr & PAGE_MASK) | flags | 1;
    __asm__ volatile ("invlpg (%0)" : : "r" (virtual_addr) : "memory");

    return 0;
}