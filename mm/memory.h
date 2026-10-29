/*
 * vmpinix Memory Management Subsystem - memory.h
 * Central Memory Manager and Hardware Page Table Interface Header
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE - 1))

typedef unsigned int pde_t;
typedef unsigned int pte_t;

int memory_subsystem_init(void);
void* mm_allocate_pages(unsigned int count);
void mm_free_pages(void *ptr, unsigned int count);
unsigned long long mm_get_total_free_ram(void);

#endif