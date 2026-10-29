/*
 * vmpinix Loop Subsystem - while.c
 * Background Polling Worker and Event Loop Poller Implementation
 */

#include "loop.h"

static volatile int polling_active = 0;

void while_polling_worker_init(void) {
    polling_active = 1;
}

void while_polling_worker_loop(void) {
    // Çekirdek boştayken veya arka planda sürekli çalışan yoklama (polling) döngüsü
    while (polling_active) {
        // Gelen G/O isteklerini veya zaman aşımına uğrayan görevleri yokla
        __asm__ volatile ("pause");
        
        // Örnek: Aktif görev yoksa döngüyü rahatlat
        if (loop_get_active_task_count() == 0) {
            __asm__ volatile ("hlt");
        }
    }
}

void while_polling_worker_stop(void) {
    polling_active = 0;
}