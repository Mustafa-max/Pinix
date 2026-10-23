/*
 * vmpinix IDE Subsystem - ide.c
 * ATA/IDE Primary and Secondary Controller PIO Mode Driver Implementation
 */

#include "block.h"

#define IDE_DATA_PORT       0x1F0
#define IDE_FEATURES_PORT   0x1F1
#define IDE_SECTOR_COUNT    0x1F2
#define IDE_LBA_LOW         0x1F3
#define IDE_LBA_MID         0x1F4
#define IDE_LBA_HIGH        0x1F5
#define IDE_DEVICE_PORT     0x1F6
#define IDE_COMMAND_PORT    0x1F7
#define IDE_STATUS_PORT     0x1F7

#define IDE_STATUS_BSY      0x80
#define IDE_STATUS_DRDY     0x40
#define IDE_STATUS_DF       0x20
#define IDE_STATUS_ERR      0x01

#define IDE_CMD_READ_PIO    0x20
#define IDE_CMD_WRITE_PIO   0x30
#define IDE_CMD_CACHE_FLUSH 0xE7

static inline void ide_outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char ide_inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void ide_outw(unsigned short port, unsigned short val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned short ide_inw(unsigned short port) {
    unsigned short ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void ide_poll(void) {
    for (int i = 0; i < 4; i++) {
        (void)ide_inb(IDE_STATUS_PORT);
    }
    while (ide_inb(IDE_STATUS_PORT) & IDE_STATUS_BSY);
    while (!(ide_inb(IDE_STATUS_PORT) & IDE_STATUS_DRDY));
}

int ide_read_sectors_pio(block_device_t *dev, unsigned long sector, unsigned int count, void *buf) {
    if (!dev || !buf || count == 0) {
        return BLOCK_ERROR_INVALID;
    }

    unsigned short *target = (unsigned short*)buf;

    for (unsigned int c = 0; c < count; c++) {
        unsigned long current_sector = sector + c;

        ide_poll();
        ide_outb(IDE_DEVICE_PORT, 0xE0 | ((current_sector >> 24) & 0x0F));
        ide_outb(IDE_FEATURES_PORT, 0x00);
        ide_outb(IDE_SECTOR_COUNT, 1);
        ide_outb(IDE_LBA_LOW, (unsigned char)(current_sector & 0xFF));
        ide_outb(IDE_LBA_MID, (unsigned char)((current_sector >> 8) & 0xFF));
        ide_outb(IDE_LBA_HIGH, (unsigned char)((current_sector >> 16) & 0xFF));
        ide_outb(IDE_COMMAND_PORT, IDE_CMD_READ_PIO);

        ide_poll();

        for (int i = 0; i < 256; i++) {
            target[i + (c * 256)] = ide_inw(IDE_DATA_PORT);
        }
    }

    return BLOCK_SUCCESS;
}

int ide_write_sectors_pio(block_device_t *dev, unsigned long sector, unsigned int count, const void *buf) {
    if (!dev || !buf || count == 0) {
        return BLOCK_ERROR_INVALID;
    }

    const unsigned short *source = (const unsigned short*)buf;

    for (unsigned int c = 0; c < count; c++) {
        unsigned long current_sector = sector + c;

        ide_poll();
        ide_outb(IDE_DEVICE_PORT, 0xE0 | ((current_sector >> 24) & 0x0F));
        ide_outb(IDE_FEATURES_PORT, 0x00);
        ide_outb(IDE_SECTOR_COUNT, 1);
        ide_outb(IDE_LBA_LOW, (unsigned char)(current_sector & 0xFF));
        ide_outb(IDE_LBA_MID, (unsigned char)((current_sector >> 8) & 0xFF));
        ide_outb(IDE_LBA_HIGH, (unsigned char)((current_sector >> 16) & 0xFF));
        ide_outb(IDE_COMMAND_PORT, IDE_CMD_WRITE_PIO);

        ide_poll();

        for (int i = 0; i < 256; i++) {
            ide_outw(IDE_DATA_PORT, source[i + (c * 256)]);
        }

        ide_outb(IDE_COMMAND_PORT, IDE_CMD_CACHE_FLUSH);
        ide_poll();
    }

    return BLOCK_SUCCESS;
}

int ide_init_device(unsigned int dev_id) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev) {
        return BLOCK_ERROR_INVALID;
    }

    dev->total_sectors = 8388608; // 4 GB Standart IDE Sınırı
    dev->sector_size = SECTOR_SIZE;
    dev->read = ide_read_sectors_pio;
    dev->write = ide_write_sectors_pio;
    dev->initialized = 1;

    return BLOCK_SUCCESS;
}