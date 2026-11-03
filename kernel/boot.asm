BITS 64
DEFAULT REL

section .text.entry
 global start
 extern kernel_main

start:
    cli
    cld
    lea rsp, [rel stack_top]
    and rsp, -16
    xor rbp, rbp
    ; Bootloader contract: RDI points to a valid pinix_boot_info_t.
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits
