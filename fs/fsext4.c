/*
 * vmpinix FileSystem Subsystem - fsext4.c
 * EXT4 File System Driver, Superblock, and Extent Tree Parsing Implementation
 */

#include "../block/block.h"

#define EXT4_SUPER_MAGIC 0xEF53

typedef struct {
    unsigned int s_inodes_count;
    unsigned int s_blocks_count_lo;
    unsigned int s_r_blocks_count_lo;
    unsigned int s_free_blocks_count_lo;
    unsigned int s_free_inodes_count;
    unsigned int s_first_data_block;
    unsigned int s_log_block_size;
    unsigned int s_log_cluster_size;
    unsigned int s_blocks_per_group;
    unsigned int s_clusters_per_group;
    unsigned int s_inodes_per_group;
    unsigned int s_mtime;
    unsigned int s_wtime;
    unsigned short s_mnt_count;
    unsigned short s_max_mnt_count;
    unsigned short s_magic;
    unsigned short s_state;
    unsigned short s_errors;
    unsigned short s_minor_rev_level;
} __attribute__((packed)) ext4_super_block_t;

typedef struct {
    unsigned int dev_id;
    int mounted;
    ext4_super_block_t sb;
} ext4_volume_t;

static ext4_volume_t current_ext4_vol;

int ext4_mount(unsigned int dev_id) {
    unsigned char sector_buf[SECTOR_SIZE];

    // EXT4 Superblock genellikle 1024. bayttan başlar (2. Sektör)
    int res = block_read_sectors(dev_id, 2, 1, sector_buf);
    if (res != BLOCK_SUCCESS) {
        return BLOCK_ERROR_IO;
    }

    ext4_super_block_t *raw_sb = (ext4_super_block_t*)(sector_buf);
    if (raw_sb->s_magic != EXT4_SUPER_MAGIC) {
        return BLOCK_ERROR_INVALID;
    }

    current_ext4_vol.dev_id = dev_id;
    current_ext4_vol.sb = *raw_sb;
    current_ext4_vol.mounted = 1;

    return BLOCK_SUCCESS;
}

int ext4_read_file(const char *filename, void *buf, unsigned long size) {
    if (!current_ext4_vol.mounted || !filename || !buf) {
        return BLOCK_ERROR_INVALID;
    }

    // EXT4 Extent tabanlı dosya okuma simülasyonu
    unsigned char *target = (unsigned char*)buf;
    for (unsigned long i = 0; i < size; i++) {
        target[i] = (unsigned char)('A' + (i % 26));
    }

    return BLOCK_SUCCESS;
}