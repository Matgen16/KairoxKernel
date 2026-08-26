[bits 64]

global gdt_flush
gdt_flush:
    ; Load the GDT pointer (passed as first argument in RDI for System V ABI)
    lgdt [rdi]

    ; Reload data segment registers with kernel data selector (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload Code Segment (CS) using a 64-bit far return
    push 0x08                 ; 0x08 = Kernel Code Segment Selector
    lea rax, [rel .reload_cs] ; Push address of target label
    push rax
    retfq                     ; Far return 64-bit (pops RIP, then CS)

.reload_cs:
    ret