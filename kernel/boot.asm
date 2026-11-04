BITS 32

%define MULTIBOOT2_BOOT_MAGIC 0x36D76289

section .multiboot
align 8
mb2_header_start:
    dd 0xE85250D6
    dd 0
    dd mb2_header_end - mb2_header_start
    dd -(0xE85250D6 + (mb2_header_end - mb2_header_start))

    dw 1
    dw 0
    dd 16
    dd 6
    dd 8
    align 8

    dw 5
    dw 0
    dd 20
    dd 1920
    dd 1080
    dd 32
    align 8

    dw 0
    dw 0
    dd 8
mb2_header_end:

section .text.entry
 global start
 extern kernel_main

start:
    cli
    cld
    cmp eax, MULTIBOOT2_BOOT_MAGIC
    jne .bad_entry
    mov [mb2_saved_magic], eax
    mov [mb2_saved_info], ebx

    ; Temporary identity map for the first 4 GiB using 2 MiB pages.
    mov eax, pdpt
    or eax, 0x3
    mov [pml4], eax
    mov dword [pml4 + 4], 0

    mov edi, pdpt
    mov eax, pd0
    mov ecx, 4
.pdpt_loop:
    mov edx, eax
    or edx, 0x3
    mov [edi], edx
    mov dword [edi + 4], 0
    add eax, 4096
    add edi, 8
    loop .pdpt_loop

    mov edi, pd0
    xor edx, edx
    mov ecx, 2048
.page_loop:
    mov eax, edx
    or eax, 0x83
    mov [edi], eax
    mov dword [edi + 4], 0
    add edx, 0x200000
    add edi, 8
    loop .page_loop

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    mov eax, pml4
    mov cr3, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    lgdt [gdt64_pointer]
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    jmp 0x08:long_mode_entry

.bad_entry:
    cli
    hlt
    jmp .bad_entry

BITS 64
long_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor ax, ax
    mov fs, ax
    mov gs, ax

    lea rsp, [rel stack_top]
    and rsp, -16
    xor rbp, rbp
    mov edi, dword [rel mb2_saved_magic]
    mov esi, dword [rel mb2_saved_info]
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

section .rodata
align 8
gdt64:
    dq 0
    dq 0x00AF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt64_end:
gdt64_pointer:
    dw gdt64_end - gdt64 - 1
    dd gdt64

section .bss
align 4096
pml4: resb 4096
pdpt: resb 4096
pd0: resb 4 * 4096
align 4
mb2_saved_magic: resd 1
mb2_saved_info: resd 1
align 16
stack_bottom: resb 16384
stack_top:
section .note.GNU-stack noalloc noexec nowrite progbits
