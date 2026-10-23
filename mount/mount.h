/*
 * vmpinix Mount Subsystem - mount.h
 * File System Mounting Table and Namespace Management Header
 */

#ifndef _MOUNT_H
#define _MOUNT_H

#define MAX_MOUNT_POINTS 16
#define MOUNT_PATH_MAX 64

typedef struct {
    char source_dev[32];
    char target_path[MOUNT_PATH_MAX];
    char fs_type[16];
    unsigned long flags;
    unsigned int dev_id;
    int active;
} mount_point_t;

int mount_subsystem_init(void);
int do_mount(const char *source, const char *target, const char *fstype, unsigned long flags, unsigned int dev_id);
int do_unmount(const char *target);
mount_point_t* mount_resolve_path(const char *path);

#endif