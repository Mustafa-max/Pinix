$(BOOT_OBJ): kernel/boot.asm | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(NASM) -f elf64 -Wall -Werror \
		-w-reloc-abs-dword -w-reloc-abs-qword \
		-o $@ $<
