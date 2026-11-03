CC ?= gcc
LD ?= ld
NASM ?= nasm
READELF ?= readelf

BUILD_DIR := build
KERNEL := $(BUILD_DIR)/pinix.elf
BOOT_OBJ := $(BUILD_DIR)/boot.o
KERNEL_OBJ := $(BUILD_DIR)/kernel_main.o

CFLAGS := -m64 -std=c11 -O2 -Wall -Wextra -Werror \
          -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -fno-pie -fno-asynchronous-unwind-tables \
          -fno-unwind-tables -mno-red-zone -mno-mmx -mno-sse \
          -mcmodel=small -nostdlib -nostdinc -Iinclude
LDFLAGS := -m elf_x86_64 -nostdlib -T kernel/linker.ld \
           -z max-page-size=0x1000

.PHONY: all clean check
all: $(KERNEL)

$(BUILD_DIR):
	mkdir -p $@

$(BOOT_OBJ): kernel/boot.asm | $(BUILD_DIR)
	$(NASM) -f elf64 -Wall -Werror -o $@ $<

$(KERNEL_OBJ): kernel/main.c include/boot_info.h include/stdint.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(BOOT_OBJ) $(KERNEL_OBJ) kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BOOT_OBJ) $(KERNEL_OBJ)

check: $(KERNEL)
	$(READELF) -h $(KERNEL) | grep -q 'Class:.*ELF64'
	$(READELF) -h $(KERNEL) | grep -q 'Machine:.*Advanced Micro Devices X86-64'
	$(READELF) -h $(KERNEL) | grep -q 'Entry point address:'

clean:
	rm -rf $(BUILD_DIR)
