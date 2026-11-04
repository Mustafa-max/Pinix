# Pinix boot and display status

Pinix is an x86-64 ELF kernel with a Multiboot2 header. It is not a BIOS boot
sector or a UEFI `.efi` application and does not include GRUB or another loader.
GRUB configured with its Multiboot2 command can load the same kernel from BIOS or
UEFI; UEFI firmware itself does not directly execute this ELF kernel.

## Build and check

Run `make`; output is `build/pinix.elf`. `make check` validates ELF64/x86-64 metadata
and uses `grub-file --is-x86-multiboot2` when that utility is installed. `make
check-multiboot2` requires `grub-file`.

The Multiboot2 header requests the memory-map and framebuffer information tags and
optionally requests 1920x1080x32. GRUB may return another mode if the firmware or
adapter cannot provide that resolution. The 32-bit entry trampoline validates the
Multiboot2 magic, creates temporary identity mappings for the first 4 GiB, switches
to long mode, establishes a stack, and passes the Multiboot2 information pointer to
`kernel_main`.

## Current functionality and limits

- COM1 serial logging and CPU vendor identification are early diagnostics.
- The kernel parses the Multiboot2 framebuffer tag and draws a blue background with
  a black bottom panel when it receives a 32-bit RGB framebuffer with supported masks.
- PCI scanning reports enumerated IDs/classes. Reported BAR values are inventory
  only; this code does not map or access GPU MMIO registers.
- This is not a native Intel/AMD GPU driver, DRM/KMS implementation, mouse cursor,
  desktop, USB/xHCI driver, storage stack, or ACPI power-management implementation.
- The temporary identity map is not a production virtual-memory manager. Physical
  framebuffer addresses outside the mapped range need a proper MMIO mapping first.
- `make check` does not prove a successful boot. Test with GRUB in QEMU BIOS and
  OVMF/UEFI, then test physical machines separately. HP/Dell are vendor names, not
  hardware interfaces; support depends on each model's firmware and devices.

No universal real-PC compatibility is claimed. A real graphics driver needs device-
specific PCI identification, BAR/resource validation and mapping, GPU initialization,
mode setting, interrupts/DMA where applicable, and per-hardware testing. The boot
framebuffer path is intentionally used as the safe fallback.
