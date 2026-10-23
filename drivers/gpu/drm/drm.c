/*
 * Pinix-Kernel DRM Subsystem - drm.c
 * Direct Rendering Manager Core Implementation with Intel UEFI GOP Integration
 */

#include "drm.h"
#include "kms.h"

#define DRM_MAX_DEVICES 8
#define DRM_MAX_CLIENTS 32
#define DRM_MAX_BUFFERS 256
#define DRM_MAGIC_BASE 1000

typedef struct {
    int id;
    unsigned int pid;
    int authenticated;
} drm_client_t;

typedef struct {
    unsigned int handle;
    unsigned long size;
    unsigned long paddr;
    int freed;
} drm_gem_object_t;

typedef struct {
    drm_device_t devices[DRM_MAX_DEVICES];
    drm_client_t clients[DRM_MAX_CLIENTS];
    drm_gem_object_t buffers[DRM_MAX_BUFFERS];
    int device_count;
    int client_count;
    int buffer_count;
    int initialized;
} drm_subsystem_t;

static drm_subsystem_t drm_subsys;

int drm_init(void) {
    if (drm_subsys.initialized) {
        return DRM_SUCCESS;
    }

    drm_subsys.device_count = 0;
    drm_subsys.client_count = 0;
    drm_subsys.buffer_count = 0;

    for (int i = 0; i < DRM_MAX_DEVICES; i++) {
        drm_subsys.devices[i].id = 0;
        drm_subsys.devices[i].mmio_base = 0;
        drm_subsys.devices[i].fb_base = 0;
        drm_subsys.devices[i].width = 0;
        drm_subsys.devices[i].height = 0;
        drm_subsys.devices[i].pitch = 0;
        drm_subsys.devices[i].bpp = 0;
        drm_subsys.devices[i].initialized = 0;
    }

    for (int i = 0; i < DRM_MAX_CLIENTS; i++) {
        drm_subsys.clients[i].id = 0;
        drm_subsys.clients[i].pid = 0;
        drm_subsys.clients[i].authenticated = 0;
    }

    for (int i = 0; i < DRM_MAX_BUFFERS; i++) {
        drm_subsys.buffers[i].handle = 0;
        drm_subsys.buffers[i].size = 0;
        drm_subsys.buffers[i].paddr = 0;
        drm_subsys.buffers[i].freed = 1;
    }

    drm_device_t primary;
    primary.id = 0;
    primary.mmio_base = 0xFEB00000; // Intel IGD MMIO Base
    primary.fb_base = 0xE0000000;   // UEFI GOP Framebuffer Base[cite: 10]
    primary.width = 1024;
    primary.height = 768;
    primary.pitch = 1024 * 4;
    primary.bpp = 32;
    primary.initialized = 1;

    drm_register_device(&primary);
    drm_subsys.initialized = 1;

    return DRM_SUCCESS;
}

void drm_exit(void) {
    if (!drm_subsys.initialized) {
        return;
    }

    for (int i = 0; i < DRM_MAX_DEVICES; i++) {
        drm_subsys.devices[i].initialized = 0;
    }

    drm_subsys.device_count = 0;
    drm_subsys.client_count = 0;
    drm_subsys.buffer_count = 0;
    drm_subsys.initialized = 0;
}

drm_device_t* drm_get_primary_device(void) {
    if (!drm_subsys.initialized || drm_subsys.device_count == 0) {
        return (drm_device_t*)0;
    }
    return &drm_subsys.devices[0];
}

int drm_register_device(drm_device_t *dev) {
    if (!dev) {
        return DRM_ERROR_INVALID_PARAM;
    }
    if (drm_subsys.device_count >= DRM_MAX_DEVICES) {
        return DRM_ERROR_NOMEM;
    }

    drm_subsys.devices[drm_subsys.device_count] = *dev;
    drm_subsys.device_count++;
    return DRM_SUCCESS;
}

int drm_unregister_device(unsigned int id) {
    int found = 0;
    for (int i = 0; i < drm_subsys.device_count; i++) {
        if (drm_subsys.devices[i].id == id) {
            found = 1;
            for (int j = i; j < drm_subsys.device_count - 1; j++) {
                drm_subsys.devices[j] = drm_subsys.devices[j + 1];
            }
            drm_subsys.device_count--;
            break;
        }
    }
    return found ? DRM_SUCCESS : DRM_ERROR_NO_DEVICE;
}

unsigned int drm_read_reg(drm_device_t *dev, unsigned long offset) {
    if (!dev || !dev->initialized) {
        return 0;
    }
    volatile unsigned int *reg = (volatile unsigned int *)(dev->mmio_base + offset);
    return *reg;
}

void drm_write_reg(drm_device_t *dev, unsigned long offset, unsigned int val) {
    if (!dev || !dev->initialized) {
        return;
    }
    volatile unsigned int *reg = (volatile unsigned int *)(dev->mmio_base + offset);
    *reg = val;
}

int drm_create_client(unsigned int pid) {
    if (drm_subsys.client_count >= DRM_MAX_CLIENTS) {
        return DRM_ERROR_NOMEM;
    }

    int idx = drm_subsys.client_count++;
    drm_subsys.clients[idx].id = idx + 1;
    drm_subsys.clients[idx].pid = pid;
    drm_subsys.clients[idx].authenticated = 0;

    return drm_subsys.clients[idx].id;
}

int drm_authenticate_client(int client_id, unsigned int magic) {
    for (int i = 0; i < drm_subsys.client_count; i++) {
        if (drm_subsys.clients[i].id == client_id) {
            if (magic == (unsigned int)(DRM_MAGIC_BASE + client_id)) {
                drm_subsys.clients[i].authenticated = 1;
                return DRM_SUCCESS;
            }
            return DRM_ERROR_INVALID_PARAM;
        }
    }
    return DRM_ERROR_NO_DEVICE;
}

int drm_gem_create_object(unsigned long size, unsigned int *handle) {
    if (size == 0 || !handle) {
        return DRM_ERROR_INVALID_PARAM;
    }

    if (drm_subsys.buffer_count >= DRM_MAX_BUFFERS) {
        return DRM_ERROR_NOMEM;
    }

    int free_idx = -1;
    for (int i = 0; i < DRM_MAX_BUFFERS; i++) {
        if (drm_subsys.buffers[i].freed) {
            free_idx = i;
            break;
        }
    }

    if (free_idx == -1) {
        return DRM_ERROR_NOMEM;
    }

    drm_subsys.buffers[free_idx].handle = (unsigned int)(free_idx + 100);
    drm_subsys.buffers[free_idx].size = size;
    drm_subsys.buffers[free_idx].paddr = 0xE0000000 + (free_idx * 0x10000);
    drm_subsys.buffers[free_idx].freed = 0;

    *handle = drm_subsys.buffers[free_idx].handle;
    return DRM_SUCCESS;
}

int drm_gem_free_object(unsigned int handle) {
    for (int i = 0; i < DRM_MAX_BUFFERS; i++) {
        if (!drm_subsys.buffers[i].freed && drm_subsys.buffers[i].handle == handle) {
            drm_subsys.buffers[i].freed = 1;
            drm_subsys.buffers[i].handle = 0;
            drm_subsys.buffers[i].size = 0;
            drm_subsys.buffers[i].paddr = 0;
            return DRM_SUCCESS;
        }
    }
    return DRM_ERROR_INVALID_PARAM;
}

int drm_prime_handle_to_fd(unsigned int handle, int *fd) {
    if (!fd) {
        return DRM_ERROR_INVALID_PARAM;
    }
    for (int i = 0; i < DRM_MAX_BUFFERS; i++) {
        if (!drm_subsys.buffers[i].freed && drm_subsys.buffers[i].handle == handle) {
            *fd = (int)(handle + 500);
            return DRM_SUCCESS;
        }
    }
    return DRM_ERROR_INVALID_PARAM;
}

int drm_prime_fd_to_handle(int fd, unsigned int *handle) {
    if (!handle) {
        return DRM_ERROR_INVALID_PARAM;
    }
    unsigned int h = (unsigned int)(fd - 500);
    for (int i = 0; i < DRM_MAX_BUFFERS; i++) {
        if (!drm_subsys.buffers[i].freed && drm_subsys.buffers[i].handle == h) {
            *handle = h;
            return DRM_SUCCESS;
        }
    }
    return DRM_ERROR_INVALID_PARAM;
}

int drm_ioctl(unsigned int request, unsigned long arg) {
    if (!drm_subsys.initialized) {
        return DRM_ERROR_NO_DEVICE;
    }

    switch (request) {
        case 0x0001:
            break;
        case 0x0002:
            break;
        case 0x0003:
            break;
        case 0x0004:
            break;
        default:
            return DRM_ERROR_INVALID_PARAM;
    }

    (void)arg;
    return DRM_SUCCESS;
}