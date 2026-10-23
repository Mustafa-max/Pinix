/*
 * vmpinix Device Subsystem - bus.c
 * Bus Management (PCI, USB, etc.) and Device-Driver Matching
 */

#include "devices.h"

#define MAX_BUS_COUNT 16

typedef struct {
    unsigned int bus_id;
    char name[32];
    unsigned int registered_devices[64];
    int device_count;
} bus_t;

static bus_t system_buses[MAX_BUS_COUNT];
static int bus_count = 0;

int bus_subsystem_init(void) {
    bus_count = 0;
    for (int i = 0; i < MAX_BUS_COUNT; i++) {
        system_buses[i].bus_id = 0;
        system_buses[i].device_count = 0;
        system_buses[i].name[0] = '\0';
    }
    return DEVICE_SUCCESS;
}

int bus_register(unsigned int bus_id, const char *name) {
    if (bus_count >= MAX_BUS_COUNT || !name) {
        return DEVICE_ERROR_NOMEM;
    }

    bus_t *bus = &system_buses[bus_count++];
    bus->bus_id = bus_id;
    bus->device_count = 0;

    int i = 0;
    while (name[i] != '\0' && i < 31) {
        bus->name[i] = name[i];
        i++;
    }
    bus->name[i] = '\0';

    return DEVICE_SUCCESS;
}

int bus_attach_device(unsigned int bus_id, unsigned int device_id) {
    for (int i = 0; i < bus_count; i++) {
        if (system_buses[i].bus_id == bus_id) {
            if (system_buses[i].device_count >= 64) {
                return DEVICE_ERROR_NOMEM;
            }
            system_buses[i].registered_devices[system_buses[i].device_count++] = device_id;
            return DEVICE_SUCCESS;
        }
    }
    return DEVICE_ERROR_NOTFOUND;
}