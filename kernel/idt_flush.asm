BITS 64
section .text

global idt_flush
idt_flush:
    lidt [rdi]
    ret

global idt_stub
idt_stub:
    cli
    hlt
    jmp $