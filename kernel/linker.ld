OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* read + execute */
    data PT_LOAD FLAGS(6); /* read + write */
}

SECTIONS
{
    /* Keep all 32-bit trampoline absolute relocations safely below 2 GiB. */
    . = 0x00100000;
    __kernel_start = .;

    .multiboot ALIGN(8) : {
        __multiboot_start = .;
        KEEP(*(.multiboot))
        __multiboot_end = .;
    } :text

    .text ALIGN(0x1000) : {
        KEEP(*(.text.entry))
        *(.text .text.*)
    } :text

    .rodata ALIGN(0x1000) : {
        *(.rodata .rodata.*)
    } :text

    .data ALIGN(0x1000) : {
        *(.data .data.*)
    } :data

    .bss ALIGN(0x1000) (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    } :data

    __kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame .eh_frame.*)
    }

    ASSERT(__multiboot_start - __kernel_start < 0x8000,
           "Multiboot2 header is not in the first 32 KiB")
    ASSERT(__kernel_end < 0x80000000,
           "kernel exceeds low 2 GiB trampoline address range")
    ASSERT(start >= __kernel_start && start < __kernel_end,
           "kernel entry is outside the loaded image")
}
