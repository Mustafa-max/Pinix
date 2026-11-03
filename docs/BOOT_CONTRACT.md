# Pinix bootloader integration contract

Pinix is an x86-64 ELF kernel, not a BIOS boot sector or a UEFI `.efi` application.
This repository intentionally contains no BIOS/UEFI bootloader. An external loader
may support either firmware path, but it must implement this contract before jumping
to the kernel.

## Kernel image and entry

- Build with `make`; the output is `build/pinix.elf` (ELF64, x86-64, linked at 1 MiB).
- Load the ELF `PT_LOAD` segments at their linked addresses, zeroing each segment's
  `p_memsz - p_filesz` tail. Preserve the ELF segment permissions where practical.
- Enter `start` in 64-bit long mode with the kernel image and its 16 KiB stack mapped
  and executable/writable as appropriate. The entry sets its own stack.
- Disable maskable interrupts before entry. The entry also executes `cli` and `cld`.
- Pass a pointer to `pinix_boot_info_t` in `RDI` using the x86-64 System V calling
  convention. The structure and all referenced arrays must remain mapped and valid
  while the kernel uses them.
- The current minimal kernel validates the information structure and halts. It does
  not yet initialize devices, memory management, a console, or userspace.

## Boot information

The ABI is defined in `include/boot_info.h`. `magic`, `version`, and `struct_size`
must be valid. Memory map entries use `pinix_memory_region_t`; entry stride is
`memory_map_entry_size`, so loaders may append fields to a larger entry. Firmware
mode is descriptive only: the kernel does not call BIOS or UEFI services.
Framebuffer fields are optional; zero address/size means no framebuffer supplied.

A BIOS-based loader must perform any required transition into long mode and provide
this same ABI. A UEFI-based loader must exit boot services before transferring
control, load the ELF, and provide this ABI (including the framebuffer if desired).
The loader must translate its native firmware data into the Pinix memory-region
format. Neither firmware mode can be promised on a machine unless a compatible
external loader is used.

## Hardware compatibility

The target is x86-64. Intel and AMD CPUs that implement the required x86-64
instruction set are architectural targets. HP and Dell are system vendors, not
separate CPU architectures; compatibility depends on the specific model, firmware,
chipset, devices, and external loader. This code does not yet contain validated
SMBIOS/ACPI detection or production hardware drivers, so it must not claim universal
HP/Dell or device support.

## Build check

`make check` verifies the generated ELF class and machine type. It does not prove
that a bootloader implements this contract or that the kernel boots on physical
hardware.
