/*
 * vmpinix Mount Subsystem - mount.c
 * Mount Table Registry, Namespace Resolution, and Volume Attachment Implementation
 */

#include "mount.h"

typedef struct {
    mount_point_t entries[MAX_MOUNT_POINTS];
    int count;
    int initialized;
} mount_table_t;

static mount_table_t m_table;

int mount_subsystem_init(void) {
    if (m_table.initialized) {
        return 0;
    }

    m_table.count = 0;
    for (int i = 0; i < MAX_MOUNT_POINTS; i++) {
        m_table.entries[i].active = 0;
        m_table.entries[i].dev_id = 0;
        m_table.entries[i].source_dev[0] = '\0';
        m_table.entries[i].target_path[0] = '\0';
        m_table.entries[i].fs_type[0] = '\0';
    }

    m_table.initialized = 1;
    return 0;
}

int do_mount(const char *source, const char *target, const char *fstype, unsigned long flags, unsigned int dev_id) {
    if (!target || !fstype || m_table.count >= MAX_MOUNT_POINTS) {
        return -1;
    }

    // Aynı hedef yol önceden bağlanmış mı kontrol et
    for (int i = 0; i < m_table.count; i++) {
        if (m_table.entries[i].active) {
            int match = 1;
            int j = 0;
            while (m_table.entries[i].target_path[j] != '\0' && target[j] != '\0') {
                if (m_table.entries[i].target_path[j] != target[j]) {
                    match = 0;
                    break;
                }
                j++;
            }
            if (match && m_table.entries[i].target_path[j] == target[j]) {
                return -2; // Zaten bağlı (Device busy)
            }
        }
    }

    mount_point_t *mp = &m_table.entries[m_table.count++];
    mp->dev_id = dev_id;
    mp->flags = flags;
    mp->active = 1;

    // Hedef yolu kopyala
    int i = 0;
    while (target[i] != '\0' && i < MOUNT_PATH_MAX - 1) {
        mp->target_path[i] = target[i];
        i++;
    }
    mp->target_path[i] = '\0';

    // Dosya sistemi türünü kopyala
    i = 0;
    if (fstype) {
        while (fstype[i] != '\0' && i < 15) {
            mp->fs_type[i] = fstype[i];
            i++;
        }
        mp->fs_type[i] = '\0';
    }

    // Kaynak aygıt adını kopyala
    i = 0;
    if (source) {
        while (source[i] != '\0' && i < 31) {
            mp->source_dev[i] = source[i];
            i++;
        }
        mp->source_dev[i] = '\0';
    }

    return 0;
}

int do_unmount(const char *target) {
    if (!target) {
        return -1;
    }

    int found_index = -1;
    for (int i = 0; i < m_table.count; i++) {
        if (m_table.entries[i].active) {
            int match = 1;
            int j = 0;
            while (m_table.entries[i].target_path[j] != '\0' && target[j] != '\0') {
                if (m_table.entries[i].target_path[j] != target[j]) {
                    match = 0;
                    break;
                }
                j++;
            }
            if (match && m_table.entries[i].target_path[j] == target[j]) {
                found_index = i;
                break;
            }
        }
    }

    if (found_index == -1) {
        return -3; // Bağlantı bulunamadı
    }

    // Tabloyu kaydırarak temizle
    m_table.entries[found_index].active = 0;
    for (int j = found_index; j < m_table.count - 1; j++) {
        m_table.entries[j] = m_table.entries[j + 1];
    }
    m_table.count--;

    return 0;
}

mount_point_t* mount_resolve_path(const char *path) {
    if (!path) return (mount_point_t*)0;

    mount_point_t *best_match = (mount_point_t*)0;
    int longest_prefix = 0;

    for (int i = 0; i < m_table.count; i++) {
        if (m_table.entries[i].active) {
            int match_len = 0;
            int j = 0;
            while (m_table.entries[i].target_path[j] != '\0' && path[j] != '\0' && m_table.entries[i].target_path[j] == path[j]) {
                match_len++;
                j++;
            }

            if (match_len > longest_prefix) {
                longest_prefix = match_len;
                best_match = &m_table.entries[i];
            }
        }
    }

    return best_match;
}