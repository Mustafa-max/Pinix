BITS 32
DEFAULT ABS

%define MB2_HEADER_MAGIC 0xE85250D6
%define MB2_BOOT_MAGIC   0x36D76289

; Multiboot2 header: keep this section within the first 32 KiB of the ELF file.
section .multiboot
align 8
mb2_header_start:
    dd MB2_HEADER_MAGIC
    dd 0
    dd mb2_header_end - mb2_header_start
    dd -(MB2_HEADER_MAGIC + (mb2_header_end - mb2_header_start))

    ; Request memory map (tag 6) and framebuffer (tag 8).
    dw 1
    dw 0
    dd 16
    dd 6
    dd 8
    align 8

    ; Prefer 1920x1080x32; this request is optional.
    dw 5
    dw 0
    dd 20
    dd 1920
    dd 1080
    dd 32
    align 8

    ; Required end tag.
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

    ; Multiboot2 i386 entry: EAX=boot magic, EBX=info structure address.
    cmp eax, MB2_BOOT_MAGIC
    jne .bad_entry
    mov [mb2_saved_magic], eax
    mov [mb2_saved_info], ebx

    ; Install our known flat 32-bit code/data descriptors before paging setup.
    lgdt [gdt32_pointer]
    jmp 0x08:.known_segments
.known_segments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Clear our PML4, PDPT and four page directories (6 * 4 KiB).
    mov edi, pml4
    xor eax, eax
    mov ecx, (6 * 4096) / 4
    rep stosd

    ; PML4[0] -> PDPT. All table addresses are below 2 GiB by linker assertion.
    mov eax, pdpt
    or eax, 0x03
    mov [pml4], eax

    ; PDPT entries point to four page directories (identity map 4 GiB total).
    mov edi, pdpt
    mov eax, pd0
    mov ecx, 4
.pdpt_loop:
    mov edx, eax
    or edx, 0x03
    mov [edi], edx
    add eax, 4096
    add edi, 8
    loop .pdpt_loop

    ; 2048 2 MiB pages identity-map physical addresses 0..4 GiB.
    mov edi, pd0
    xor edx, edx
    mov ecx, 2048
.page_loop:
    mov eax, edx
    or eax, 0x083              ; present, writable, page-size bit
    mov [edi], eax
    add edx, 0x200000
    add edi, 8
    loop .page_loop

    ; Enable PAE and PSE, set the paging root and enable long mode.
    mov eax, cr4
    or eax, (1 << 5) | (1 << 4)
    mov cr4, eax

    mov eax, pml4
    mov cr3, eax

    mov ecx, 0xC0000080        ; EFER MSR
    rdmsr
    or eax, (1 << 8)           ; EFER.LME
    wrmsr

    lgdt [gdt64_pointer]
    mov eax, cr0
    or eax, (1 << 31)          ; CR0.PG; PE is already set by Multiboot2
    mov cr0, eax
    jmp 0x18:long_mode_entry

.bad_entry:
    cli
    hlt
    jmp .bad_entry

BITS 64
long_mode_entry:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Establish SysV x86-64 stack alignment before calling C.
    mov rsp, stack_top
    and rsp, -16
    xor rbp, rbp

    ; kernel_main(uint32_t multiboot_magic, uint32_t mbi_address)
    mov edi, dword [abs mb2_saved_magic]
    mov esi, dword [abs mb2_saved_info]
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

section .rodata
align 8
gdt64:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF    ; 32-bit protected-mode code selector 0x08
    dq 0x00CF92000000FFFF    ; flat data selector 0x10
    dq 0x00AF9A000000FFFF    ; 64-bit code selector 0x18
    dq 0x00CF92000000FFFF    ; data selector 0x20
gdt64_end:

gdt32_pointer:
    dw gdt64_end - gdt64 - 1
    dd gdt64

gdt64_pointer:
    dw gdt64_end - gdt64 - 1
    dd gdt64

section .bss
; The linker aligns this section to 4 KiB. Avoid ALIGN/ALIGNB here so NASM
; versions which treat BSS alignment specially do not emit initialized padding.
pml4: resb 4096
pdpt: resb 4096
pd0:  resb 4 * 4096
mb2_saved_magic: resd 1
mb2_saved_info:  resd 1
; 8 bytes of padding make the following stack 16-byte aligned.
resb 8
stack_bottom: resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits
