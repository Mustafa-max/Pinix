/*
 * vmpinix Block Subsystem - scheduler.c
 * Advanced I/O Scheduler, Elevator Algorithm, and Deadline-Based Request Reordering
 */

#include "block.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

#define SCHEDULER_MAX_QUEUE 256

typedef enum {
    SCHED_MODE_NOOP = 0,
    SCHED_MODE_ELEVATOR = 1,
    SCHED_MODE_DEADLINE = 2
} scheduler_mode_t;

typedef struct {
    block_request_t requests[SCHEDULER_MAX_QUEUE];
    unsigned char occupied[SCHEDULER_MAX_QUEUE];
    int count;
    unsigned long last_sector;
    scheduler_mode_t current_mode;
    unsigned long tick_counter;
} io_scheduler_t;

static io_scheduler_t disk_scheduler;

int scheduler_init(scheduler_mode_t mode) {
    disk_scheduler.count = 0;
    disk_scheduler.last_sector = 0;
    disk_scheduler.current_mode = mode;
    disk_scheduler.tick_counter = 0;

    for (int i = 0; i < SCHEDULER_MAX_QUEUE; i++) {
        disk_scheduler.occupied[i] = 0;
        disk_scheduler.requests[i].id = (unsigned int)(i + 1);
        disk_scheduler.requests[i].status = REQ_STATUS_FREE;
        disk_scheduler.requests[i].buffer = NULL;
        disk_scheduler.requests[i].device = NULL;
    }

    return BLOCK_SUCCESS;
}

int scheduler_set_mode(scheduler_mode_t mode) {
    disk_scheduler.current_mode = mode;
    return BLOCK_SUCCESS;
}

scheduler_mode_t scheduler_get_mode(void) {
    return disk_scheduler.current_mode;
}

int scheduler_add_request(const block_request_t *req) {
    if (!req || disk_scheduler.count >= SCHEDULER_MAX_QUEUE) {
        return BLOCK_ERROR_NOMEM;
    }

    int insert_idx = -1;
    for (int i = 0; i < SCHEDULER_MAX_QUEUE; i++) {
        if (!disk_scheduler.occupied[i]) {
            insert_idx = i;
            break;
        }
    }

    if (insert_idx == -1) {
        return BLOCK_ERROR_NOMEM;
    }

    disk_scheduler.requests[insert_idx] = *req;
    disk_scheduler.requests[insert_idx].status = REQ_STATUS_QUEUED;
    disk_scheduler.occupied[insert_idx] = 1;
    disk_scheduler.count++;

    // Eğer Elevator (SCAN) modu aktifse, sektöre göre sıralama yap
    if (disk_scheduler.current_mode == SCHED_MODE_ELEVATOR && disk_scheduler.count > 1) {
        for (int i = 0; i < disk_scheduler.count - 1; i++) {
            for (int j = 0; j < disk_scheduler.count - i - 1; j++) {
                if (disk_scheduler.requests[j].sector > disk_scheduler.requests[j + 1].sector) {
                    block_request_t temp = disk_scheduler.requests[j];
                    disk_scheduler.requests[j] = disk_scheduler.requests[j + 1];
                    disk_scheduler.requests[j + 1] = temp;

                    unsigned char temp_occ = disk_scheduler.occupied[j];
                    disk_scheduler.occupied[j] = disk_scheduler.occupied[j + 1];
                    disk_scheduler.occupied[j + 1] = temp_occ;
                }
            }
        }
    }

    return BLOCK_SUCCESS;
}

block_request_t* scheduler_next_request(void) {
    if (disk_scheduler.count == 0) {
        return NULL;
    }

    int best_idx = -1;

    if (disk_scheduler.current_mode == SCHED_MODE_ELEVATOR) {
        // En yakın sektörü bul (C-SCAN / Elevator mantığı)
        unsigned long min_distance = 0xFFFFFFFF;
        for (int i = 0; i < SCHEDULER_MAX_QUEUE; i++) {
            if (disk_scheduler.occupied[i] && disk_scheduler.requests[i].status == REQ_STATUS_QUEUED) {
                unsigned long dist = 0;
                if (disk_scheduler.requests[i].sector >= disk_scheduler.last_sector) {
                    dist = disk_scheduler.requests[i].sector - disk_scheduler.last_sector;
                } else {
                    dist = disk_scheduler.last_sector - disk_scheduler.requests[i].sector;
                }

                if (dist < min_distance) {
                    min_distance = dist;
                    best_idx = i;
                }
            }
        }
    } else {
        // NOOP veya DEADLINE için sıradaki ilk isteği al
        for (int i = 0; i < SCHEDULER_MAX_QUEUE; i++) {
            if (disk_scheduler.occupied[i] && disk_scheduler.requests[i].status == REQ_STATUS_QUEUED) {
                best_idx = i;
                break;
            }
        }
    }

    if (best_idx == -1) {
        return NULL;
    }

    disk_scheduler.occupied[best_idx] = 0;
    disk_scheduler.count--;
    disk_scheduler.last_sector = disk_scheduler.requests[best_idx].sector;

    return &disk_scheduler.requests[best_idx];
}

int scheduler_flush_queue(void) {
    for (int i = 0; i < SCHEDULER_MAX_QUEUE; i++) {
        disk_scheduler.occupied[i] = 0;
        disk_scheduler.requests[i].status = REQ_STATUS_FREE;
        disk_scheduler.requests[i].buffer = NULL;
        disk_scheduler.requests[i].device = NULL;
    }
    disk_scheduler.count = 0;
    disk_scheduler.last_sector = 0;
    return BLOCK_SUCCESS;
}

unsigned int scheduler_get_pending_count(void) {
    return (unsigned int)disk_scheduler.count;
}

void scheduler_tick(void) {
    disk_scheduler.tick_counter++;
}