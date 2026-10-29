/*
 * vmpinix Block Device Subsystem - block.c
 * Comprehensive Block Layer Management, Request Queues, and Device Driver Dispatcher
 */

#include "block.h"

#define NULL ((void*)0)

static block_subsystem_t block_subsys;

static int default_dummy_read(block_device_t *dev, unsigned long sector, unsigned int count, void *buf) {
    (void)dev;
    (void)sector;
    (void)count;
    if (!buf) return BLOCK_ERROR_INVALID;
    unsigned char *p = (unsigned char*)buf;
    for (unsigned int i = 0; i < (count * SECTOR_SIZE); i++) {
        p[i] = 0;
    }
    return BLOCK_SUCCESS;
}

static int default_dummy_write(block_device_t *dev, unsigned long sector, unsigned int count, const void *buf) {
    (void)dev;
    (void)sector;
    (void)count;
    (void)buf;
    return BLOCK_SUCCESS;
}

int block_subsystem_init(void) {
    if (block_subsys.initialized) {
        return BLOCK_SUCCESS;
    }

    block_subsys.device_count = 0;
    block_subsys.request_head = 0;
    block_subsys.request_tail = 0;
    block_subsys.request_count = 0;

    for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
        block_subsys.devices[i].id = 0;
        block_subsys.devices[i].total_sectors = 0;
        block_subsys.devices[i].sector_size = SECTOR_SIZE;
        block_subsys.devices[i].read = default_dummy_read;
        block_subsys.devices[i].write = default_dummy_write;
        block_subsys.devices[i].initialized = 0;
    }

    for (int i = 0; i < MAX_REQUEST_QUEUE; i++) {
        block_subsys.requests[i].id = (unsigned int)i;
        block_subsys.requests[i].status = REQ_STATUS_FREE;
        block_subsys.requests[i].buffer = NULL;
        block_subsys.requests[i].device = NULL;
    }

    block_device_t root_disk;
    root_disk.id = 1;
    root_disk.total_sectors = 2097152; // 1 GB Disk
    root_disk.sector_size = SECTOR_SIZE;
    root_disk.read = default_dummy_read;
    root_disk.write = default_dummy_write;
    root_disk.initialized = 1;
    root_disk.name[0] = 's';
    root_disk.name[1] = 'd';
    root_disk.name[2] = 'a';
    root_disk.name[3] = '\0';

    block_register_device(&root_disk);
    block_subsys.initialized = 1;

    return BLOCK_SUCCESS;
}

void block_subsystem_exit(void) {
    if (!block_subsys.initialized) {
        return;
    }
    for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
        block_subsys.devices[i].initialized = 0;
    }
    block_subsys.device_count = 0;
    block_subsys.request_count = 0;
    block_subsys.initialized = 0;
}

int block_register_device(block_device_t *dev) {
    if (!dev || !block_subsys.initialized) {
        return BLOCK_ERROR_INVALID;
    }
    if (block_subsys.device_count >= MAX_BLOCK_DEVICES) {
        return BLOCK_ERROR_NOMEM;
    }

    int idx = block_subsys.device_count++;
    block_subsys.devices[idx] = *dev;
    block_subsys.devices[idx].initialized = 1;

    return BLOCK_SUCCESS;
}

int block_unregister_device(unsigned int id) {
    int found = 0;
    for (int i = 0; i < block_subsys.device_count; i++) {
        if (block_subsys.devices[i].id == id) {
            found = 1;
            for (int j = i; j < block_subsys.device_count - 1; j++) {
                block_subsys.devices[j] = block_subsys.devices[j + 1];
            }
            block_subsys.device_count--;
            break;
        }
    }
    return found ? BLOCK_SUCCESS : BLOCK_ERROR_INVALID;
}

block_device_t* block_get_device(unsigned int id) {
    for (int i = 0; i < block_subsys.device_count; i++) {
        if (block_subsys.devices[i].id == id) {
            return &block_subsys.devices[i];
        }
    }
    return NULL;
}

int block_read_sectors(unsigned int dev_id, unsigned long sector, unsigned int count, void *buf) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev || !dev->initialized || !buf) {
        return BLOCK_ERROR_INVALID;
    }
    if (sector + count > dev->total_sectors) {
        return BLOCK_ERROR_IO;
    }
    return dev->read(dev, sector, count, buf);
}

int block_write_sectors(unsigned int dev_id, unsigned long sector, unsigned int count, const void *buf) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev || !dev->initialized || !buf) {
        return BLOCK_ERROR_INVALID;
    }
    if (sector + count > dev->total_sectors) {
        return BLOCK_ERROR_IO;
    }
    return dev->write(dev, sector, count, buf);
}

int block_submit_request(unsigned int dev_id, request_type_t type, unsigned long sector, unsigned int count, void *buf) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev || !dev->initialized || !buf) {
        return BLOCK_ERROR_INVALID;
    }

    if (block_subsys.request_count >= MAX_REQUEST_QUEUE) {
        return BLOCK_ERROR_NOMEM;
    }

    int tail = block_subsys.request_tail;
    block_request_t *req = &block_subsys.requests[tail];

    req->type = type;
    req->sector = sector;
    req->sector_count = count;
    req->buffer = buf;
    req->status = REQ_STATUS_QUEUED;
    req->device = dev;

    block_subsys.request_tail = (block_subsys.request_tail + 1) % MAX_REQUEST_QUEUE;
    block_subsys.request_count++;

    return BLOCK_SUCCESS;
}

int block_process_request_queue(void) {
    if (!block_subsys.initialized || block_subsys.request_count == 0) {
        return 0;
    }

    int processed = 0;
    while (block_subsys.request_count > 0) {
        int head = block_subsys.request_head;
        block_request_t *req = &block_subsys.requests[head];

        if (req->status == REQ_STATUS_QUEUED) {
            req->status = REQ_STATUS_IN_PROGRESS;
            int res = 0;

            if (req->type == REQ_TYPE_READ) {
                res = req->device->read(req->device, req->sector, req->sector_count, req->buffer);
            } else {
                res = req->device->write(req->device, req->sector, req->sector_count, req->buffer);
            }

            if (res == BLOCK_SUCCESS) {
                req->status = REQ_STATUS_COMPLETED;
            } else {
                req->status = REQ_STATUS_ERROR;
            }
            processed++;
        }

        block_subsys.request_head = (block_subsys.request_head + 1) % MAX_REQUEST_QUEUE;
        block_subsys.request_count--;
    }

    return processed;
}

unsigned long long block_get_device_capacity(unsigned int dev_id) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev || !dev->initialized) {
        return 0;
    }
    return dev->total_sectors * (unsigned long long)dev->sector_size;
}

int block_flush_device(unsigned int dev_id) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev || !dev->initialized) {
        return BLOCK_ERROR_INVALID;
    }
    return BLOCK_SUCCESS;
}

int block_verify_sector_bounds(unsigned int dev_id, unsigned long sector, unsigned int count) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev || !dev->initialized) {
        return BLOCK_ERROR_INVALID;
    }
    if (sector + count > dev->total_sectors) {
        return BLOCK_ERROR_IO;
    }
    return BLOCK_SUCCESS;
}
