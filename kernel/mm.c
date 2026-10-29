/*
 * vmpinix Core Kernel - mm.c
 * Kernel Heap Allocator, Virtual-Physical Bridge, and FS/Disk Integration
 */

#include "mm.h"

// pmm_free_page için harici prototip bildirimi (dosya bulma hatasını önler)
extern void pmm_free_page(void *ptr);

// mm/ ve fs/ katmanlarına köprüler
extern void* ram_allocate_frame(void);
extern int block_write_sectors(unsigned int dev_id, unsigned long sector, unsigned int count, const void *buf);
extern int block_read_sectors(unsigned int dev_id, unsigned long sector, unsigned int count, void *buf);

int kernel_mm_subsystem_init(void) {
    return 0;
}

// Çekirdek içi basit dinamik bellek tahsisi (Heap / kmalloc simülasyonu)
void* mm_allocate_kernel_pages(unsigned int count) { (void)count; return (void*)0; }
unsigned long long mm_get_free_memory_bytes(void) { return 0; }
void mm_free_kernel_pages(void *ptr, unsigned int count) { (void)ptr; (void)count; }
int memory_subsystem_init(void) { return 0; }

void* kernel_kmalloc(size_t size) {
    if (size == 0) return (void*)0;
    
    // 4KB ve altı talepler için fiziksel RAM çerçevesi tahsis et
    return ram_allocate_frame();
}

void kernel_kfree(void *ptr) {
    if (!ptr) return;
    pmm_free_page(ptr);
}

// Bellekteki sıkışan veya boştaki sayfaları fs/ ve disk blok katmanı üzerinden diske yazma (Swap / Disk Bridge)
int kernel_swap_to_disk(void *virtual_addr, unsigned int dev_id, unsigned long sector) {
    if (!virtual_addr) {
        return -1;
    }

    // 1 sayfalık (4096 bayt = 8 sektör x 512 bayt) veriyi diske yaz
    int res = block_write_sectors(dev_id, sector, 8, virtual_addr);
    return res;
}