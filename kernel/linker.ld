OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0x00100000;
    __kernel_start = .;

    /* GRUB scans the first 32 KiB of the image for this header. */
    .multiboot ALIGN(8) : {
        KEEP(*(.multiboot))
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

    ASSERT(start >= __kernel_start && start < __kernel_end,
           "kernel entry is outside the loaded image")
}
