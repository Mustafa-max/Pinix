/*
 * vmpinix Device Subsystem - devices.c
 * Core Device Manager and Device Tree Implementation
 */

#include "devices.h"

#define MAX_SYSTEM_DEVICES 256

typedef struct {
    device_t devices[MAX_SYSTEM_DEVICES];
    int count;
    int initialized;
} device_manager_t;

static device_manager_t dev_manager;

int devices_subsystem_init(void) {
    if (dev_manager.initialized) {
        return DEVICE_SUCCESS;
    }

    dev_manager.count = 0;
    for (int i = 0; i < MAX_SYSTEM_DEVICES; i++) {
        dev_manager.devices[i].id = 0;
        dev_manager.devices[i].initialized = 0;
        dev_manager.devices[i].driver_data = (void*)0;
        dev_manager.devices[i].parent = (device_t*)0;
    }

    dev_manager.initialized = 1;
    return DEVICE_SUCCESS;
}

int device_register(device_t *dev) {
    if (!dev || !dev_manager.initialized) {
        return DEVICE_ERROR_INVALID;
    }
    if (dev_manager.count >= MAX_SYSTEM_DEVICES) {
        return DEVICE_ERROR_NOMEM;
    }

    int idx = dev_manager.count++;
    dev_manager.devices[idx] = *dev;
    dev_manager.devices[idx].initialized = 1;

    return DEVICE_SUCCESS;
}

int device_unregister(unsigned int id) {
    int found = 0;
    for (int i = 0; i < dev_manager.count; i++) {
        if (dev_manager.devices[i].id == id) {
            found = 1;
            for (int j = i; j < dev_manager.count - 1; j++) {
                dev_manager.devices[j] = dev_manager.devices[j + 1];
            }
            dev_manager.count--;
            break;
        }
    }
    return found ? DEVICE_SUCCESS : DEVICE_ERROR_NOTFOUND;
}

device_t* device_get(unsigned int id) {
    for (int i = 0; i < dev_manager.count; i++) {
        if (dev_manager.devices[i].id == id) {
            return &dev_manager.devices[i];
        }
    }
    return (device_t*)0;
}