#ifndef PINIX_PCI_H
#define PINIX_PCI_H

#include "../../include/stdint.h"

#define PINIX_PCI_MAX_DEVICES 256u
#define PINIX_PCI_BAR_COUNT 6u

#define PINIX_PCI_BAR_NONE   0u
#define PINIX_PCI_BAR_MMIO32 1u
#define PINIX_PCI_BAR_MMIO64 2u
#define PINIX_PCI_BAR_IO     3u

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t header_type;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t programming_interface;
    uint8_t revision;
    uint64_t bar[PINIX_PCI_BAR_COUNT];
    uint8_t bar_type[PINIX_PCI_BAR_COUNT];
} pinix_pci_device_t;

/* Enumerates PCI functions via x86 PCI configuration mechanism #1 (0xCF8/0xCFC).
 * BAR values are reported as assigned resources; they are not mapped or accessed.
 */
uint32_t pinix_pci_scan(void);
const pinix_pci_device_t *pinix_pci_get_devices(uint32_t *count);

#endif
