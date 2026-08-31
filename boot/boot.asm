BITS 32

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

section .bss
align 4096
global pml4_table
pml4_table: resb 4096
pdpt_table: resb 4096
pd_table:   resb 4096

align 16
stack_bottom:
    resb 16384 ; 16 KB stack
stack_top:

align 4
multiboot_ptr: resd 1 ; physical address of the Multiboot info struct

section .text
global _start
_start:
    cli
    mov [multiboot_ptr], ebx ; GRUB leaves the Multiboot info ptr in ebx
    mov esp, stack_top
    call setup_page_tables
    jmp enter_long_mode
.hang:
    hlt
    jmp .hang

setup_page_tables:
    mov eax, pdpt_table
    or eax, 0x3
    mov [pml4_table], eax
    mov dword [pml4_table + 4], 0

    mov eax, pd_table
    or eax, 0x3
    mov [pdpt_table], eax
    mov dword [pdpt_table + 4], 0

    mov ecx, 0
.map_pd_entry:
    mov eax, 0x200000
    mul ecx
    or eax, 0x83
    mov [pd_table + ecx*8], eax
    mov dword [pd_table + ecx*8 + 4], 0
    inc ecx
    cmp ecx, 4
    jl .map_pd_entry
    ret

enter_long_mode:
    lgdt [gdt64.pointer]

    mov eax, cr4
    or eax, 1<<5
    mov cr4, eax

    mov eax, pml4_table
    mov cr3, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1<<8
    wrmsr

    mov eax, cr0
    or eax, 1<<31
    mov cr0, eax

    jmp gdt64.code_seg:long_mode_start

section .rodata
gdt64:
    dq 0
.code_seg: equ $ - gdt64
    dq (1<<41) | (1<<43) | (1<<44) | (1<<47) | (1<<53)
.data_seg: equ $ - gdt64
    dq (1<<41) | (1<<44) | (1<<47)
.end:
.pointer:
    dw gdt64.end - gdt64 - 1
    dd dword gdt64

section .text
BITS 64
extern kernel_main

long_mode_start:
    cli
    mov ax, gdt64.data_seg
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, stack_top

    mov edi, [multiboot_ptr] ; 32-bit mov zero-extends into rdi (arg 1, SysV ABI)
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang