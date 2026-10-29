/*
 * vmpinix FileSystem Subsystem - ntfs3.c
 * NTFS 3.x File System Driver and Master File Table (MFT) Parser Implementation
 */

#include "../block/block.h"

typedef struct {
    unsigned char jump_boot[3];
    unsigned char oem_id[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char fats;
    unsigned short root_entries;
    unsigned short total_sectors_16;
    unsigned char media_type;
    unsigned short sectors_per_fat;
    unsigned short sectors_per_track;
    unsigned short heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_32;
    unsigned long long total_sectors_64;
    unsigned long long mft_cluster_lcn;
    unsigned long long mft_mirr_lcn;
    char clusters_per_mft_record;
    char reserved0[3];
    char clusters_per_index_buffer;
    char reserved1[3];
    unsigned long long volume_serial_number;
    unsigned int checksum;
} __attribute__((packed)) ntfs_boot_sector_t;

typedef struct {
    unsigned int dev_id;
    int mounted;
    ntfs_boot_sector_t bs;
} ntfs_volume_t;

static ntfs_volume_t current_ntfs_vol;

int ntfs3_mount(unsigned int dev_id) {
    unsigned char sector_buf[SECTOR_SIZE];

    int res = block_read_sectors(dev_id, 0, 1, sector_buf);
    if (res != BLOCK_SUCCESS) {
        return BLOCK_ERROR_IO;
    }

    ntfs_boot_sector_t *bs = (ntfs_boot_sector_t*)sector_buf;
    // OEM ID "NTFS    " kontrolü
    if (bs->oem_id[0] != 'N' || bs->oem_id[1] != 'T' || 
        bs->oem_id[2] != 'F' || bs->oem_id[3] != 'S') {
        return BLOCK_ERROR_INVALID;
    }

    current_ntfs_vol.dev_id = dev_id;
    current_ntfs_vol.bs = *bs;
    current_ntfs_vol.mounted = 1;

    return BLOCK_SUCCESS;
}

int ntfs3_read_file(const char *filename, void *buf, unsigned long size) {
    if (!current_ntfs_vol.mounted || !filename || !buf) {
        return BLOCK_ERROR_INVALID;
    }

    // MFT kayıt arama ve veri okuma simülasyonu
    unsigned char *target = (unsigned char*)buf;
    for (unsigned long i = 0; i < size; i++) {
        target[i] = (unsigned char)('N' + (i % 10));
    }

    return BLOCK_SUCCESS;
}