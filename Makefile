CC = gcc
LD = ld
NASM = nasm

CFLAGS = -m64 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -ffreestanding -fno-pie -I. -I./include

BUILD_DIR = build
TARGET = vmpinix

SRC = $(shell find . -name "*.c" \
	! -path "./arch/32bit/*" \
	! -path "./arch/86x/*")
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(BUILD_DIR)/boot.o $(OBJ)
	$(LD) -m elf_x86_64 -n --entry=start -o $(TARGET) $(BUILD_DIR)/boot.o $(OBJ)

$(BUILD_DIR)/boot.o: kernel/boot.asm
	mkdir -p $(BUILD_DIR)
	$(NASM) -f elf64 -o $@ $<

%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	find . -name "*.o" -delete
	find . -name "*.obj" -delete

.PHONY: all clean