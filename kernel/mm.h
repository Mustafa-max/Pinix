/*
 * vmpinix Core Kernel - mm.h
 * Kernel Memory Management and Subsystem Bridge Header
 */

#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include "../include/stdint.h"

int kernel_mm_subsystem_init(void);
void* kernel_kmalloc(size_t size);
void kernel_kfree(void *ptr);
int kernel_swap_to_disk(void *virtual_addr, unsigned int dev_id, unsigned long sector);

#endif