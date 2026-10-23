/*
 * vmpinix Block Device Subsystem - block.h
 * Comprehensive Block Layer Header Definitions
 */

#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SUCCESS 0
#define BLOCK_ERROR_INVALID -1
#define BLOCK_ERROR_NOMEM -2
#define BLOCK_ERROR_IO -3

#define SECTOR_SIZE 512
#define MAX_BLOCK_DEVICES 32
#define MAX_REQUEST_QUEUE 256

typedef enum {
    REQ_TYPE_READ = 0,
    REQ_TYPE_WRITE = 1
} request_type_t;

typedef enum {
    REQ_STATUS_FREE = 0,
    REQ_STATUS_QUEUED = 1,
    REQ_STATUS_IN_PROGRESS = 2,
    REQ_STATUS_COMPLETED = 3,
    REQ_STATUS_ERROR = 4
} request_status_t;

typedef struct block_device block_device_t;

typedef struct {
    unsigned int id;
    request_type_t type;
    unsigned long sector;
    unsigned int sector_count;
    void *buffer;
    request_status_t status;
    block_device_t *device;
} block_request_t;

struct block_device {
    unsigned int id;
    char name[32];
    unsigned long long total_sectors;
    unsigned int sector_size;
    int (*read)(block_device_t *dev, unsigned long sector, unsigned int count, void *buf);
    int (*write)(block_device_t *dev, unsigned long sector, unsigned int count, const void *buf);
    int initialized;
};

// Derleyicinin aradığı eksik yapı tanımı burada eklendi:
typedef struct {
    block_device_t devices[MAX_BLOCK_DEVICES];
    int device_count;
    block_request_t requests[MAX_REQUEST_QUEUE];
    int request_head;
    int request_tail;
    int request_count;
    int initialized;
} block_subsystem_t;

int block_subsystem_init(void);
void block_subsystem_exit(void);
int block_register_device(block_device_t *dev);
int block_unregister_device(unsigned int id);
block_device_t* block_get_device(unsigned int id);
int block_read_sectors(unsigned int dev_id, unsigned long sector, unsigned int count, void *buf);
int block_write_sectors(unsigned int dev_id, unsigned long sector, unsigned int count, const void *buf);
int block_submit_request(unsigned int dev_id, request_type_t type, unsigned long sector, unsigned int count, void *buf);
int block_process_request_queue(void);
unsigned long long block_get_device_capacity(unsigned int dev_id);
int block_flush_device(unsigned int dev_id);
int block_verify_sector_bounds(unsigned int dev_id, unsigned long sector, unsigned int count);

#endif