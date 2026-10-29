/*
 * vmpinix Device Subsystem - devices.h
 * Core Device Management and Hierarchy Definitions
 */

#ifndef DEVICES_H
#define DEVICES_H

#define DEVICE_SUCCESS 0
#define DEVICE_ERROR_INVALID -1
#define DEVICE_ERROR_NOMEM -2
#define DEVICE_ERROR_NOTFOUND -3

typedef enum {
    DEV_TYPE_BLOCK = 0,
    DEV_TYPE_CHAR = 1,
    DEV_TYPE_NETWORK = 2,
    DEV_TYPE_DISPLAY = 3,
    DEV_TYPE_BUS = 4
} device_type_t;

typedef struct device device_t;

struct device {
    unsigned int id;
    char name[32];
    device_type_t type;
    unsigned int bus_id;
    unsigned int class_id;
    int (*read)(device_t *dev, void *buf, unsigned long size, unsigned long offset);
    int (*write)(device_t *dev, const void *buf, unsigned long size, unsigned long offset);
    int (*control)(device_t *dev, unsigned int cmd, unsigned long arg);
    void *driver_data;
    device_t *parent;
    int initialized;
};

int devices_subsystem_init(void);
int device_register(device_t *dev);
int device_unregister(unsigned int id);
device_t* device_get(unsigned int id);

#endif