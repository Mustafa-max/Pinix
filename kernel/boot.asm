global start
extern kernel_main

section .text
bits 64

start:
    cli
    mov rsp, stack_top
    call kernel_main

.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: