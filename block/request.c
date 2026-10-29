/*
 * vmpinix Block Subsystem - request.c
 * Advanced Block I/O Request Management, Allocation, and Queue Merging Engine
 */

#include "block.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

#define MAX_CONCURRENT_REQUESTS 512

typedef struct {
    block_request_t request_pool[MAX_CONCURRENT_REQUESTS];
    unsigned char allocation_map[MAX_CONCURRENT_REQUESTS / 8];
    unsigned int active_count;
    unsigned int total_submitted;
    unsigned int total_completed;
    unsigned int total_failed;
} request_manager_t;

static request_manager_t req_manager;

int request_subsystem_init(void) {
    req_manager.active_count = 0;
    req_manager.total_submitted = 0;
    req_manager.total_completed = 0;
    req_manager.total_failed = 0;

    for (int i = 0; i < (MAX_CONCURRENT_REQUESTS / 8); i++) {
        req_manager.allocation_map[i] = 0;
    }

    for (int i = 0; i < MAX_CONCURRENT_REQUESTS; i++) {
        req_manager.request_pool[i].id = (unsigned int)(i + 1);
        req_manager.request_pool[i].type = REQ_TYPE_READ;
        req_manager.request_pool[i].sector = 0;
        req_manager.request_pool[i].sector_count = 0;
        req_manager.request_pool[i].buffer = NULL;
        req_manager.request_pool[i].status = REQ_STATUS_FREE;
        req_manager.request_pool[i].device = NULL;
    }

    return BLOCK_SUCCESS;
}

block_request_t* request_allocate(unsigned int dev_id, request_type_t type, unsigned long sector, unsigned int count, void *buffer) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev || !buffer || count == 0) {
        return NULL;
    }

    if (req_manager.active_count >= MAX_CONCURRENT_REQUESTS) {
        return NULL;
    }

    int free_index = -1;
    for (int i = 0; i < MAX_CONCURRENT_REQUESTS; i++) {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        if (!(req_manager.allocation_map[byte_idx] & (1 << bit_idx))) {
            req_manager.allocation_map[byte_idx] |= (1 << bit_idx);
            free_index = i;
            break;
        }
    }

    if (free_index == -1) {
        return NULL;
    }

    block_request_t *req = &req_manager.request_pool[free_index];
    req->type = type;
    req->sector = sector;
    req->sector_count = count;
    req->buffer = buffer;
    req->status = REQ_STATUS_QUEUED;
    req->device = dev;

    req_manager.active_count++;
    req_manager.total_submitted++;

    return req;
}

int request_free(block_request_t *req) {
    if (!req) {
        return BLOCK_ERROR_INVALID;
    }

    int index = -1;
    for (int i = 0; i < MAX_CONCURRENT_REQUESTS; i++) {
        if (&req_manager.request_pool[i] == req) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return BLOCK_ERROR_INVALID;
    }

    int byte_idx = index / 8;
    int bit_idx = index % 8;

    if (!(req_manager.allocation_map[byte_idx] & (1 << bit_idx))) {
        return BLOCK_ERROR_INVALID; // Zaten boş
    }

    req_manager.allocation_map[byte_idx] &= ~(1 << bit_idx);
    req->status = REQ_STATUS_FREE;
    req->buffer = NULL;
    req->device = NULL;

    if (req_manager.active_count > 0) {
        req_manager.active_count--;
    }

    return BLOCK_SUCCESS;
}

int request_dispatch(block_request_t *req) {
    if (!req || req->status != REQ_STATUS_QUEUED) {
        return BLOCK_ERROR_INVALID;
    }

    req->status = REQ_STATUS_IN_PROGRESS;
    int res = BLOCK_SUCCESS;

    if (req->type == REQ_TYPE_READ) {
        res = req->device->read(req->device, req->sector, req->sector_count, req->buffer);
    } else if (req->type == REQ_TYPE_WRITE) {
        res = req->device->write(req->device, req->sector, req->sector_count, req->buffer);
    } else {
        res = BLOCK_ERROR_INVALID;
    }

    if (res == BLOCK_SUCCESS) {
        req->status = REQ_STATUS_COMPLETED;
        req_manager.total_completed++;
    } else {
        req->status = REQ_STATUS_ERROR;
        req_manager.total_failed++;
    }

    return res;
}

int request_merge_check(block_request_t *req1, block_request_t *req2) {
    if (!req1 || !req2) {
        return 0;
    }

    if (req1->device != req2->device || req1->type != req2->type) {
        return 0;
    }

    // ardışık sektörler mi kontrol et
    if (req1->sector + req1->sector_count == req2->sector) {
        return 1;
    }

    return 0;
}

int request_merge(block_request_t *req1, block_request_t *req2) {
    if (!request_merge_check(req1, req2)) {
        return BLOCK_ERROR_INVALID;
    }

    req1->sector_count += req2->sector_count;
    request_free(req2);

    return BLOCK_SUCCESS;
}

unsigned int request_get_active_count(void) {
    return req_manager.active_count;
}

unsigned int request_get_total_submitted(void) {
    return req_manager.total_submitted;
}

unsigned int request_get_total_completed(void) {
    return req_manager.total_completed;
}

unsigned int request_get_total_failed(void) {
    return req_manager.total_failed;
}