/*
 * vmpinix Partition Subsystem - partition.c
 * MBR and GPT Partition Table Parser and Management Implementation
 */

#include "block.h"

#define MBR_BOOT_SIGNATURE 0xAA55
#define PARTITION_MAX_ENTRIES 128

typedef struct {
    unsigned char drive_attributes;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    unsigned int start_lba;
    unsigned int sector_count;
} __attribute__((packed)) mbr_partition_record_t;

typedef struct {
    unsigned char bootstrap_code[446];
    mbr_partition_record_t partitions[4];
    unsigned short boot_signature;
} __attribute__((packed)) mbr_sector_t;

typedef struct {
    unsigned char partition_type_guid[16];
    unsigned char unique_partition_guid[16];
    unsigned long long starting_lba;
    unsigned long long ending_lba;
    unsigned long long attributes;
    unsigned short partition_name[36];
} __attribute__((packed)) gpt_partition_entry_t;

typedef struct {
    char signature[8];
    unsigned int revision;
    unsigned int header_size;
    unsigned int header_crc32;
    unsigned int reserved;
    unsigned long long current_lba;
    unsigned long long backup_lba;
    unsigned long long first_usable_lba;
    unsigned long long last_usable_lba;
    unsigned char disk_guid[16];
    unsigned long long partition_entry_lba;
    unsigned int number_of_partition_entries;
    unsigned int size_of_partition_entry;
    unsigned int partition_entry_array_crc32;
} __attribute__((packed)) gpt_header_t;

typedef struct {
    unsigned int id;
    unsigned int parent_device_id;
    unsigned long long start_sector;
    unsigned long long sector_count;
    int is_gpt;
    char name[32];
} partition_t;

#define MAX_TOTAL_PARTITIONS 64
static partition_t partition_table[MAX_TOTAL_PARTITIONS];
static int partition_count = 0;

int partition_init(void) {
    partition_count = 0;
    for (int i = 0; i < MAX_TOTAL_PARTITIONS; i++) {
        partition_table[i].id = 0;
        partition_table[i].parent_device_id = 0;
        partition_table[i].start_sector = 0;
        partition_table[i].sector_count = 0;
        partition_table[i].is_gpt = 0;
        partition_table[i].name[0] = '\0';
    }
    return BLOCK_SUCCESS;
}

int partition_parse_mbr(unsigned int dev_id) {
    unsigned char sector_buf[SECTOR_SIZE];
    int res = block_read_sectors(dev_id, 0, 1, sector_buf);
    if (res != BLOCK_SUCCESS) {
        return BLOCK_ERROR_IO;
    }

    mbr_sector_t *mbr = (mbr_sector_t*)sector_buf;
    if (mbr->boot_signature != MBR_BOOT_SIGNATURE) {
        return BLOCK_ERROR_INVALID;
    }

    for (int i = 0; i < 4; i++) {
        mbr_partition_record_t *rec = &mbr->partitions[i];
        if (rec->partition_type == 0) {
            continue;
        }

        if (partition_count >= MAX_TOTAL_PARTITIONS) {
            break;
        }

        partition_t *part = &partition_table[partition_count++];
        part->id = (unsigned int)(partition_count);
        part->parent_device_id = dev_id;
        part->start_sector = rec->start_lba;
        part->sector_count = rec->sector_count;
        part->is_gpt = 0;

        part->name[0] = 'p';
        part->name[1] = (char)('0' + i + 1);
        part->name[2] = '\0';
    }

    return BLOCK_SUCCESS;
}

int partition_parse_gpt(unsigned int dev_id) {
    unsigned char sector_buf[SECTOR_SIZE];
    int res = block_read_sectors(dev_id, 1, 1, sector_buf); // LBA 1: GPT Header
    if (res != BLOCK_SUCCESS) {
        return BLOCK_ERROR_IO;
    }

    gpt_header_t *gpt = (gpt_header_t*)sector_buf;
    if (gpt->signature[0] != 'E' || gpt->signature[1] != 'F' ||
        gpt->signature[2] != 'I' || gpt->signature[3] != ' ' ||
        gpt->signature[4] != 'P' || gpt->signature[5] != 'A' ||
        gpt->signature[6] != 'R' || gpt->signature[7] != 'T') {
        return BLOCK_ERROR_INVALID;
    }

    unsigned long long entry_lba = gpt->partition_entry_lba;
    unsigned int num_entries = gpt->number_of_partition_entries;
    unsigned int entry_size = gpt->size_of_partition_entry;

    if (num_entries > 128) num_entries = 128; // Güvenlik sınırı

    unsigned char entry_buf[SECTOR_SIZE];
    unsigned int entries_per_sector = SECTOR_SIZE / entry_size;

    for (unsigned int i = 0; i < num_entries; i++) {
        unsigned long long sector_offset = entry_lba + (i / entries_per_sector);
        unsigned int sector_idx = i % entries_per_sector;

        res = block_read_sectors(dev_id, (unsigned long)sector_offset, 1, entry_buf);
        if (res != BLOCK_SUCCESS) {
            continue;
        }

        gpt_partition_entry_t *entry = (gpt_partition_entry_t*)(entry_buf + (sector_idx * entry_size));

        // Boş partition GUID kontrolü (tümü 0 ise boştur)
        int is_empty = 1;
        for (int b = 0; b < 16; b++) {
            if (entry->partition_type_guid[b] != 0) {
                is_empty = 0;
                break;
            }
        }

        if (is_empty) {
            continue;
        }

        if (partition_count >= MAX_TOTAL_PARTITIONS) {
            break;
        }

        partition_t *part = &partition_table[partition_count++];
        part->id = (unsigned int)(partition_count);
        part->parent_device_id = dev_id;
        part->start_sector = entry->starting_lba;
        part->sector_count = entry->ending_lba - entry->starting_lba + 1;
        part->is_gpt = 1;

        part->name[0] = 'g';
        part->name[1] = 'p';
        part->name[2] = 't';
        part->name[3] = '\0';
    }

    return BLOCK_SUCCESS;
}

int partition_scan_device(unsigned int dev_id) {
    // Önce MBR kontrol et, korumalı MBR ise GPT tara
    unsigned char sector_buf[SECTOR_SIZE];
    if (block_read_sectors(dev_id, 0, 1, sector_buf) != BLOCK_SUCCESS) {
        return BLOCK_ERROR_IO;
    }

    mbr_sector_t *mbr = (mbr_sector_t*)sector_buf;
    if (mbr->boot_signature != MBR_BOOT_SIGNATURE) {
        return BLOCK_ERROR_INVALID;
    }

    // Koruyucu MBR türü (EEh) ise GPT yapısını incele
    if (mbr->partitions[0].partition_type == 0xEE) {
        return partition_parse_gpt(dev_id);
    } else {
        return partition_parse_mbr(dev_id);
    }
}

partition_t* partition_get_info(unsigned int index) {
    if (index >= (unsigned int)partition_count) {
        return (partition_t*)0;
    }
    return &partition_table[index];
}

int partition_get_count(void) {
    return partition_count;
}