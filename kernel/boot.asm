BITS 32

%define MULTIBOOT2_HEADER_MAGIC 0xE85250D6
%define MULTIBOOT2_BOOT_MAGIC   0x36D76289

section .multiboot
align 8
mb2_header_start:
    dd MULTIBOOT2_HEADER_MAGIC
    dd 0                          ; architecture: i386 protected mode
    dd mb2_header_end - mb2_header_start
    dd -(MULTIBOOT2_HEADER_MAGIC + (mb2_header_end - mb2_header_start))

    ; Optional information requests: memory map (6), framebuffer (8).
    dw 1                          ; information-request header tag
    dw 0                          ; optional
    dd 16
    dd 6
    dd 8
    align 8

    ; Ask GRUB for 1920x1080x32 when the firmware/adapter supports it.
    dw 5                          ; framebuffer header tag
    dw 0                          ; optional; loader may choose a supported mode
    dd 20
    dd 1920
    dd 1080
    dd 32
    align 8

    dw 0                          ; end tag
    dw 0
    dd 8
mb2_header_end:

section .text.entry
 global start
 extern kernel_main

start:
    ; Multiboot2 i386 entry: EAX=boot magic, EBX=physical info pointer.
    cli
    cld
    cmp eax, MULTIBOOT2_BOOT_MAGIC
    jne .bad_entry
    mov [mb2_saved_magic], eax
    mov [mb2_saved_info], ebx

    ; Build identity mappings for the first 4 GiB with 2 MiB pages.
    ; This is an early-boot map, not the kernel's final virtual memory layout.
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
    or eax, 0x83                 ; present, writable, 2 MiB page
    mov [edi], eax
    mov dword [edi + 4], 0
    add edx, 0x200000
    add edi, 8
    loop .page_loop

    mov eax, cr4
    or eax, 1 << 5               ; PAE
    mov cr4, eax
    mov eax, pml4
    mov cr3, eax

    mov ecx, 0xC0000080          ; EFER
    rdmsr
    or eax, 1 << 8               ; LME
    wrmsr

    lgdt [gdt64_pointer]
    mov eax, cr0
    or eax, 1 << 31              ; paging (protected mode already active)
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
    dq 0x0000000000000000
    dq 0x00AF9A000000FFFF       ; 64-bit kernel code
    dq 0x00CF92000000FFFF       ; kernel data
gdt64_end:
gdt64_pointer:
    dw gdt64_end - gdt64 - 1
    dd gdt64

section .bss
align 4096
pml4: resb 4096
pdpt: resb 4096
pd0:  resb 4 * 4096

align 4
mb2_saved_magic: resd 1
mb2_saved_info:  resd 1

align 16
stack_bottom: resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits
