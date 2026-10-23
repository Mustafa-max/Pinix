/*
 * vmpinix Device Subsystem - devfs.c
 * Virtual Device File System (/dev) Node Management Implementation
 */

#include "devices.h"

#define MAX_DEVFS_NODES 64

typedef struct {
    char filename[32];
    unsigned int device_id;
    int active;
} devfs_node_t;

static devfs_node_t devfs_nodes[MAX_DEVFS_NODES];
static int devfs_node_count = 0;

int devfs_init(void) {
    devfs_node_count = 0;
    for (int i = 0; i < MAX_DEVFS_NODES; i++) {
        devfs_nodes[i].device_id = 0;
        devfs_nodes[i].active = 0;
        devfs_nodes[i].filename[0] = '\0';
    }
    return DEVICE_SUCCESS;
}

int devfs_register_node(const char *name, unsigned int device_id) {
    if (!name || devfs_node_count >= MAX_DEVFS_NODES) {
        return DEVICE_ERROR_NOMEM;
    }

    devfs_node_t *node = &devfs_nodes[devfs_node_count++];
    node->device_id = device_id;
    node->active = 1;

    int i = 0;
    while (name[i] != '\0' && i < 31) {
        node->filename[i] = name[i];
        i++;
    }
    node->filename[i] = '\0';

    return DEVICE_SUCCESS;
}

device_t* devfs_lookup(const char *name) {
    if (!name) return (device_t*)0;

    for (int i = 0; i < devfs_node_count; i++) {
        if (devfs_nodes[i].active) {
            // Basit string karşılaştırma
            int match = 1;
            int j = 0;
            while (devfs_nodes[i].filename[j] != '\0' && name[j] != '\0') {
                if (devfs_nodes[i].filename[j] != name[j]) {
                    match = 0;
                    break;
                }
                j++;
            }
            if (match && devfs_nodes[i].filename[j] == name[j]) {
                return device_get(devfs_nodes[i].device_id);
            }
        }
    }
    return (device_t*)0;
}