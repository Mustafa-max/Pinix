#include "pci.h"

#define PCI_CONFIG_ADDRESS 0x0CF8u
#define PCI_CONFIG_DATA    0x0CFCu

static pinix_pci_device_t devices[PINIX_PCI_MAX_DEVICES];
static uint32_t device_count;

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function,
                           uint8_t offset) {
    uint32_t address = 0x80000000u |
                       ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) |
                       ((uint32_t)function << 8) |
                       ((uint32_t)offset & 0xFCu);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static void read_bars(pinix_pci_device_t *device) {
    uint8_t header = (uint8_t)(device->header_type & 0x7Fu);
    uint8_t bar_limit = header == 0u ? 6u : (header == 1u ? 2u : 0u);

    for (uint8_t index = 0; index < bar_limit; ++index) {
        uint8_t offset = (uint8_t)(0x10u + index * 4u);
        uint32_t low = pci_read32(device->bus, device->device,
                                  device->function, offset);

        if (low == 0u || low == 0xFFFFFFFFu) {
            continue;
        }

        if ((low & 1u) != 0u) {
            device->bar[index] = (uint64_t)(low & ~3u);
            device->bar_type[index] = PINIX_PCI_BAR_IO;
            continue;
        }

        uint32_t memory_type = (low >> 1) & 3u;
        if (memory_type == 2u && index + 1u < bar_limit) {
            uint32_t high = pci_read32(device->bus, device->device,
                                       device->function,
                                       (uint8_t)(offset + 4u));
            device->bar[index] = ((uint64_t)high << 32) |
                                 (uint64_t)(low & ~0xFu);
            device->bar_type[index] = PINIX_PCI_BAR_MMIO64;
            ++index; /* upper half of the 64-bit BAR is not another resource */
        } else {
            device->bar[index] = (uint64_t)(low & ~0xFu);
            device->bar_type[index] = PINIX_PCI_BAR_MMIO32;
        }
    }
}

static void scan_function(uint8_t bus, uint8_t slot, uint8_t function) {
    uint32_t id = pci_read32(bus, slot, function, 0x00u);
    uint32_t class_info;
    uint32_t header_info;
    pinix_pci_device_t *device;

    if ((id & 0xFFFFu) == 0xFFFFu || device_count >= PINIX_PCI_MAX_DEVICES) {
        return;
    }

    device = &devices[device_count++];
    device->bus = bus;
    device->device = slot;
    device->function = function;
    device->vendor_id = (uint16_t)(id & 0xFFFFu);
    device->device_id = (uint16_t)(id >> 16);

    class_info = pci_read32(bus, slot, function, 0x08u);
    device->revision = (uint8_t)(class_info & 0xFFu);
    device->programming_interface = (uint8_t)(class_info >> 8);
    device->subclass = (uint8_t)(class_info >> 16);
    device->class_code = (uint8_t)(class_info >> 24);

    header_info = pci_read32(bus, slot, function, 0x0Cu);
    device->header_type = (uint8_t)(header_info >> 16);
    for (uint32_t i = 0; i < PINIX_PCI_BAR_COUNT; ++i) {
        device->bar[i] = 0;
        device->bar_type[i] = PINIX_PCI_BAR_NONE;
    }
    read_bars(device);
}

uint32_t pinix_pci_scan(void) {
    device_count = 0;

    for (uint32_t bus = 0; bus < 256u; ++bus) {
        for (uint32_t slot = 0; slot < 32u; ++slot) {
            uint32_t id = pci_read32((uint8_t)bus, (uint8_t)slot, 0u, 0x00u);
            if ((id & 0xFFFFu) == 0xFFFFu) {
                continue;
            }

            scan_function((uint8_t)bus, (uint8_t)slot, 0u);
            uint32_t header = pci_read32((uint8_t)bus, (uint8_t)slot,
                                         0u, 0x0Cu);
            if (((header >> 16) & 0x80u) != 0u) {
                for (uint32_t function = 1; function < 8u; ++function) {
                    scan_function((uint8_t)bus, (uint8_t)slot,
                                  (uint8_t)function);
                }
            }
        }
    }
    return device_count;
}

const pinix_pci_device_t *pinix_pci_get_devices(uint32_t *count) {
    if (count != (uint32_t *)0) {
        *count = device_count;
    }
    return devices;
}
