# Pinix bootloader integration contract

Pinix is an x86-64 ELF kernel, not a BIOS boot sector or a UEFI `.efi` application.
This repository intentionally contains no BIOS/UEFI bootloader. An external loader
must implement this contract before jumping to the kernel.

## Kernel image and entry

- Build with `make`; output: `build/pinix.elf` (ELF64, x86-64, linked at 1 MiB).
- Load the ELF `PT_LOAD` segments at their linked addresses and zero the `p_memsz -
  p_filesz` tails.
- Enter `start` in 64-bit long mode, with the image/stack mapped, interrupts disabled,
  and the x86-64 System V ABI. `start` sets a private 16 KiB stack.
- Pass a pointer to `pinix_boot_info_t` in `RDI`. Keep the structure, memory map, and
  referenced framebuffer accessible at the addresses supplied until the kernel exits.
- `framebuffer_address` must be a kernel-accessible virtual address, not an unmapped
  physical address. Current drawing supports 32-bit BGRX only.

## Discovery and drivers

`drivers/check/pci.c` scans PCI configuration mechanism #1 and records vendor/device
IDs, class codes, and firmware-assigned BAR addresses. BARs are inventory data only:
the kernel does not dereference or map them. This initial scan is not a complete
hardware inventory and is not PCI driver binding. ACPI/SMBIOS, PCIe extended config,
USB/xHCI, interrupt routing, DMA, storage, and GPU-specific drivers remain to be done.

The repository also has bounded-polled COM1 output, validated drawing to a bootloader-
provided 32-bit BGRX framebuffer, and optional legacy PS/2 keyboard/mouse initialization.
The framebuffer is not a native GPU/DRM driver, and PS/2 does not cover USB input.

## Firmware and hardware

A BIOS loader must perform any required transition to long mode and produce this ABI.
A UEFI loader must finish firmware boot services before entering the kernel and map
ELF segments/framebuffer for kernel access. The kernel does not call BIOS or UEFI
services. The firmware mode field is descriptive; it does not itself provide firmware
compatibility.

The target is x86-64; Intel/AMD are processor targets. HP/Dell model support depends
on actual hardware and firmware and must be tested per model. The kernel currently
halts after early discovery; there is no desktop, cursor, USB stack, interrupt-driven
input, reboot/shutdown UI, or guaranteed 1920x1080 mode.

## Testing

`make check` checks ELF64 and x86-64 metadata only. BIOS/UEFI QEMU/VirtualBox tests
still require an external loader translating its protocol data into the Pinix ABI.
This repository does not yet include a bootable ISO or loader.
