CC ?= gcc
LD ?= ld
NASM ?= nasm
READELF ?= readelf
GRUB_FILE ?= grub-file

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
KERNEL := $(BUILD_DIR)/vmpinix-7.0.0generic
BOOT_OBJ := $(OBJ_DIR)/kernel/boot.o

# Recursively find every C translation unit in every repository directory.
# Only generated output and Git metadata are excluded.
C_SOURCES := $(shell find . -type f -name '*.c' \
             -not -path './build/*' -not -path './.git/*' | sort)
C_OBJECTS := $(patsubst ./%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))

# Link only the early-boot kernel components. Every other discovered C file is
# still compiled into its own object by the default target.
KERNEL_C_SOURCES := $(shell find . -type f -name '*.c' \
                    -not -path './build/*' -not -path './.git/*' | sort)
KERNEL_OBJECTS := $(addprefix $(OBJ_DIR)/,$(KERNEL_C_SOURCES:.c=.o))

CFLAGS := -m64 -std=gnu11 -O2 -Wall -Wextra \
          -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -fno-pie -fno-asynchronous-unwind-tables \
          -fno-unwind-tables -mno-red-zone -mno-mmx -mno-sse \
          -mcmodel=small -nostdlib -nostdinc -Iinclude -I.
LDFLAGS := -m elf_x86_64 -nostdlib -T kernel/linker.ld \
           -z max-page-size=0x1000

.PHONY: all all-c clean check check-multiboot2
all: all-c $(KERNEL)

# Compiles all discovered C files, but does not indiscriminately link them all
# into one kernel (many directories contain separate or alternative components).
all-c: $(C_OBJECTS)

$(BUILD_DIR):
	mkdir -p $@

# Preserve each source path under build/obj, e.g. drivers/gpu/foo.c ->
# build/obj/drivers/gpu/foo.o. This prevents basename collisions.
$(OBJ_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BOOT_OBJ): kernel/boot.asm | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(NASM) -f elf64 -Wall -Werror \
		-w-reloc-abs-dword -w-reloc-abs-qword -w-reloc-rel-dword \
		-o $@ $<


$(KERNEL): $(BOOT_OBJ) $(KERNEL_OBJECTS) kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BOOT_OBJ) $(KERNEL_OBJECTS)

check: all
	$(READELF) -h $(KERNEL) | grep -q 'Class:.*ELF64'
	$(READELF) -h $(KERNEL) | grep -q 'Machine:.*Advanced Micro Devices X86-64'
	@if command -v $(GRUB_FILE) >/dev/null 2>&1; then \
		$(GRUB_FILE) --is-x86-multiboot2 $(KERNEL); \
	else \
		echo 'grub-file unavailable; skipped Multiboot2 header validation'; \
	fi

check-multiboot2: $(KERNEL)
	$(GRUB_FILE) --is-x86-multiboot2 $(KERNEL)

clean:
	rm -rf $(BUILD_DIR)
