/*
 * vmpinix SATA Subsystem - sata.c
 * AHCI (Advanced Host Controller Interface) SATA Driver Implementation
 */

#include "block.h"

#define SATA_SIG_ATA   0x00000101
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_SEMB  0xC33C0101
#define SATA_SIG_PM    0x96690101

#define AHCI_DEV_NULL  0
#define AHCI_DEV_SATA  1
#define AHCI_DEV_SEMB  2
#define AHCI_DEV_PM    3
#define AHCI_DEV_SATAPI 4

typedef struct {
    unsigned int clb;
    unsigned int clbu;
    unsigned int fb;
    unsigned int fbu;
    unsigned int is;
    unsigned int ie;
    unsigned int cmd;
    unsigned int rsv0;
    unsigned int tfd;
    unsigned int sig;
    unsigned int ssts;
    unsigned int sctl;
    unsigned int serr;
    unsigned int sact;
    unsigned int ci;
    unsigned int sntf;
    unsigned int fbs;
    unsigned int rsv1[11];
    unsigned int vendor[4];
} __attribute__((packed)) ahci_hba_port_t;

typedef struct {
    unsigned int cap;
    unsigned int ghc;
    unsigned int is;
    unsigned int pi;
    unsigned int vs;
    unsigned int ccc_ctl;
    unsigned int ccc_pts;
    unsigned int em_loc;
    unsigned int em_ctl;
    unsigned int cap2;
    unsigned int bohc;
    unsigned char rsv[0xA0 - 0x2C];
    ahci_hba_port_t ports[32];
} __attribute__((packed)) ahci_hba_memory_t;

static volatile ahci_hba_memory_t *ahci_abar = (volatile ahci_hba_memory_t*)0xFE800000;

int sata_read_sectors_ahci(block_device_t *dev, unsigned long sector, unsigned int count, void *buf) {
    if (!dev || !buf || count == 0) {
        return BLOCK_ERROR_INVALID;
    }

    // AHCI/SATA DMA okuma simülasyonu ve bellek kopyalama
    unsigned char *target = (unsigned char*)buf;
    unsigned char *source_sim = (unsigned char*)(0xFE900000 + (sector * SECTOR_SIZE));

    for (unsigned int i = 0; i < (count * SECTOR_SIZE); i++) {
        target[i] = source_sim[i];
    }

    return BLOCK_SUCCESS;
}

int sata_write_sectors_ahci(block_device_t *dev, unsigned long sector, unsigned int count, const void *buf) {
    if (!dev || !buf || count == 0) {
        return BLOCK_ERROR_INVALID;
    }

    // AHCI/SATA DMA yazma simülasyonu
    const unsigned char *source = (const unsigned char*)buf;
    unsigned char *target_sim = (unsigned char*)(0xFE900000 + (sector * SECTOR_SIZE));

    for (unsigned int i = 0; i < (count * SECTOR_SIZE); i++) {
        target_sim[i] = source[i];
    }

    return BLOCK_SUCCESS;
}

int sata_init_device(unsigned int dev_id, unsigned int port_index) {
    block_device_t *dev = block_get_device(dev_id);
    if (!dev) {
        return BLOCK_ERROR_INVALID;
    }

    if (port_index >= 32) {
        return BLOCK_ERROR_INVALID;
    }

    volatile ahci_hba_port_t *port = &ahci_abar->ports[port_index];
    unsigned int sig = port->sig;

    if (sig != SATA_SIG_ATA) {
        return BLOCK_ERROR_INVALID; // Sadece standart SATA diskler
    }

    dev->total_sectors = 104857600; // 50 GB SATA Disk Kapasitesi
    dev->sector_size = SECTOR_SIZE;
    dev->read = sata_read_sectors_ahci;
    dev->write = sata_write_sectors_ahci;
    dev->initialized = 1;

    return BLOCK_SUCCESS;
}