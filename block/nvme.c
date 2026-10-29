/*
 * vmpinix NVMe Subsystem - nvme.c
 * Non-Volatile Memory Express (NVMe) Controller and Queue Driver Implementation
 */

#include "block.h"

#define NVMe_REG_CAP      0x00
#define NVMe_REG_VS       0x08
#define NVMe_REG_CC       0x14
#define NVMe_REG_CSTS     0x1C
#define NVMe_REG_AQA      0x24
#define NVMe_REG_ASQ      0x28
#define NVMe_REG_ACQ      0x30

#define NVMe_CC_ENABLE    (1 << 0)
#define NVMe_CSTS_RDY     (1 << 0)

typedef struct {
    unsigned int doorbell_stride;
    unsigned char reserved[4088];
} __attribute__((packed)) nvme_controller_registers_t;

typedef struct {
    unsigned int command[16];
} __attribute__((packed)) nvme_submission_entry_t;

typedef struct {
    unsigned int command_id;
    unsigned int reserved0;
    unsigned short status;
    unsigned short phase_tag_and_sq_id;
    unsigned short sq_head;
    unsigned short reserved1;
} __attribute__((packed)) nvme_completion_entry_t;

static volatile nvme_controller_registers_t *nvme_bar = (volatile nvme_controller_registers_t*)0xFA000000;

int nvme_read_sectors_async(block_device_t *dev, unsigned long sector, unsigned int count, void *buf) {
    if (!dev || !buf || count == 0) {
        return BLOCK_ERROR_INVALID;
    }

    // NVMe PRP tabanlı DMA okuma simülasyonu
    unsigned char *target = (unsigned char*)buf;
    unsigned char *source_sim = (unsigned char*)(0xFA800000 + (sector * SECTOR_SIZE));

    for (unsigned int i = 0; i < (count * SECTOR_SIZE); i++) {
        target[i] = source_sim[i];
    }

    return BLOCK_SUCCESS;
}

int nvme_write_sectors_async(block_device_t *dev, unsigned long sector, unsigned int count, const void *buf) {
    if (!dev || !buf || count == 0) {
        return BLOCK_ERROR_INVALID;
    }

    // NVMe PRP tabanlı DMA yazma simülasyonu
    const unsigned char *source = (const unsigned char*)buf;
    unsigned char *target_sim = (unsigned char*)(0xFA800000 + (sector * SECTOR_SIZE));

    for (unsigned int i = 0; i < (count * SECTOR_SIZE); i++) {
        target_sim[i] = source[i];
    }

    return BLOCK_SUCCESS;
}

int nvme_init_controller(void) {
    // NVMe kontrolcü aktif etme ve bellek eşleme hazırlığı
    volatile unsigned int *cc = (volatile unsigned int*)((unsigned char*)nvme_bar + NVMe_REG_CC);
    volatile unsigned int *csts = (volatile unsigned int*)((unsigned char*)nvme_bar + NVMe_REG_CSTS);

    *cc &= ~NVMe_CC_ENABLE;
    while (*csts & NVMe_CSTS_RDY);

    // Kuyruk boyutları ve taban adresleri ayarlanır
    volatile unsigned int *aqa = (volatile unsigned int*)((unsigned char*)nvme_bar + NVMe_REG_AQA);
    *aqa = (31 << 16) | 31; // Admin Submission & Completion Queue Size = 32

    *cc |= NVMe_CC_ENABLE;
    while (!(*csts & NVMe_CSTS_RDY));

    return BLOCK_SUCCESS;
}

int nvme_init_device(unsigned int dev_id) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev) {
        return BLOCK_ERROR_INVALID;
    }

    if (nvme_init_controller() != BLOCK_SUCCESS) {
        return BLOCK_ERROR_IO;
    }

    dev->total_sectors = 419430400; // 200 GB NVMe SSD Kapasitesi
    dev->sector_size = SECTOR_SIZE;
    dev->read = nvme_read_sectors_async;
    dev->write = nvme_write_sectors_async;
    dev->initialized = 1;

    return BLOCK_SUCCESS;
}