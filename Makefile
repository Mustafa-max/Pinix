CC ?= gcc
LD ?= ld
NASM ?= nasm
READELF ?= readelf
GRUB_FILE ?= grub-file

BUILD_DIR := build
KERNEL := $(BUILD_DIR)/pinix.elf
OBJECTS := \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel_main.o \
	$(BUILD_DIR)/serial.o \
	$(BUILD_DIR)/framebuffer.o \
	$(BUILD_DIR)/ps2.o \
	$(BUILD_DIR)/pci.o

CFLAGS := -m64 -std=c11 -O2 -Wall -Wextra -Werror \
          -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -fno-pie -fno-asynchronous-unwind-tables \
          -fno-unwind-tables -mno-red-zone -mno-mmx -mno-sse \
          -mcmodel=small -nostdlib -nostdinc -Iinclude
LDFLAGS := -m elf_x86_64 -nostdlib -T kernel/linker.ld \
           -z max-page-size=0x1000

.PHONY: all clean check check-multiboot2
all: $(KERNEL)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/boot.o: kernel/boot.asm | $(BUILD_DIR)
	$(NASM) -f elf64 -Wall -Werror -o $@ $<

$(BUILD_DIR)/kernel_main.o: kernel/main.c include/boot_info.h include/stdint.h drivers/video/framebuffer.h drivers/serial/serial.h drivers/input/ps2.h drivers/check/pci.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/serial.o: drivers/serial/serial.c drivers/serial/serial.h include/stdint.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/framebuffer.o: drivers/video/framebuffer.c drivers/video/framebuffer.h include/boot_info.h include/stdint.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ps2.o: drivers/input/ps2.c drivers/input/ps2.h include/stdint.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pci.o: drivers/check/pci.c drivers/check/pci.h include/stdint.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJECTS) kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

check: $(KERNEL)
	$(READELF) -h $(KERNEL) | grep -q 'Class:.*ELF64'
	$(READELF) -h $(KERNEL) | grep -q 'Machine:.*Advanced Micro Devices X86-64'
	$(READELF) -h $(KERNEL) | grep -q 'Entry point address:'
	@if command -v $(GRUB_FILE) >/dev/null 2>&1; then \
		$(GRUB_FILE) --is-x86-multiboot2 $(KERNEL); \
	else \
		echo 'grub-file unavailable; skipped Multiboot2 header validation'; \
	fi

check-multiboot2: $(KERNEL)
	$(GRUB_FILE) --is-x86-multiboot2 $(KERNEL)

clean:
	rm -rf $(BUILD_DIR)
