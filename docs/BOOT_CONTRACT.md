# Pinix bootloader integration contract

Pinix is an x86-64 ELF kernel, not a BIOS boot sector or a UEFI `.efi` application.
This repository intentionally contains no BIOS/UEFI bootloader. An external loader
must implement this contract before jumping to the kernel.

## Kernel image and entry

- Build with `make`; output: `build/pinix.elf` (ELF64, x86-64, linked at 1 MiB).
- Load the ELF `PT_LOAD` segments at their linked addresses and zero the `p_memsz -
  p_filesz` tails.
- Enter `start` in 64-bit long mode, with the image/stack mapped, interrupts disabled,
  and the x86-64 System V ABI. `start` sets a private 16 KiB stack and calls C.
- Pass a pointer to `pinix_boot_info_t` in `RDI`. Keep the structure, memory map, and
  referenced framebuffer accessible at the addresses supplied until the kernel exits.
- `framebuffer_address` must be a kernel-accessible virtual address, not an unmapped
  physical address. Current drawing supports 32-bit BGRX only.

## Firmware and hardware

A BIOS loader must perform any required transition to long mode and produce this ABI.
A UEFI loader must finish firmware boot services before entering the kernel and map
ELF segments/framebuffer for kernel access. The kernel does not call BIOS or UEFI
services. The firmware mode field is descriptive; it does not provide firmware
compatibility by itself.

The repository currently includes bounded-polled COM1 serial output, a validated
boot-framebuffer fill/rectangle driver, and optional polling access to legacy PS/2
keyboard/mouse controllers. PS/2 is frequently absent on modern systems; many UEFI
systems expose USB keyboards through xHCI, which is not implemented. The framebuffer
is supplied/configured by the external loader; this is not a native GPU driver.

There is not yet a physical memory allocator, interrupt/IDT handler set, USB/xHCI,
PCI enumeration, NVMe/AHCI storage, ACPI/SMBIOS parsing, or broad GPU support. Thus
this is a hardware bring-up stage, not a complete desktop kernel. Intel/AMD x86-64
are CPU targets, but HP/Dell model compatibility requires testing each model and its
actual chipset/devices. No universal hardware support is claimed.

## Testing

`make check` checks ELF64 and x86-64 metadata only. QEMU/VirtualBox BIOS and UEFI
boot tests require an external loader that translates its protocol data into this
Pinix boot ABI; this repository does not yet include a bootable ISO or loader.
