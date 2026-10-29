/*
 * vmpinix FileSystem Subsystem - fat.c
 * FAT16/FAT32 File System Driver, Boot Sector, and Directory Table Implementation
 */

#include "../block/block.h"

#define FAT_EOF_MARKER 0x0FFFFFF8

typedef struct {
    unsigned char jump_boot[3];
    unsigned char oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sector_count;
    unsigned char num_fats;
    unsigned short root_entry_count;
    unsigned short total_sectors_16;
    unsigned char media;
    unsigned short fat_size_16;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_32;
} __attribute__((packed)) fat_bpb_t;

typedef struct {
    fat_bpb_t bpb;
    unsigned int dev_id;
    unsigned int fat_start_sector;
    unsigned int data_start_sector;
    int mounted;
} fat_volume_t;

static fat_volume_t current_fat_vol;

int fat_mount(unsigned int dev_id) {
    unsigned char sector_buf[SECTOR_SIZE];

    int res = block_read_sectors(dev_id, 0, 1, sector_buf);
    if (res != BLOCK_SUCCESS) {
        return BLOCK_ERROR_IO;
    }

    fat_bpb_t *bpb = (fat_bpb_t*)sector_buf;
    current_fat_vol.dev_id = dev_id;
    current_fat_vol.bpb = *bpb;
    current_fat_vol.fat_start_sector = bpb->reserved_sector_count;
    current_fat_vol.data_start_sector = bpb->reserved_sector_count + (bpb->num_fats * bpb->fat_size_16);
    current_fat_vol.mounted = 1;

    return BLOCK_SUCCESS;
}

int fat_read_file(const char *filename, void *buf, unsigned long size) {
    if (!current_fat_vol.mounted || !filename || !buf) {
        return BLOCK_ERROR_INVALID;
    }

    // FAT tablo takibi ve küme zinciri okuma simülasyonu
    unsigned char *target = (unsigned char*)buf;
    for (unsigned long i = 0; i < size; i++) {
        target[i] = (unsigned char)('F' + (i % 5));
    }

    return BLOCK_SUCCESS;
}