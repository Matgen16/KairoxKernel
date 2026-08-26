ENTRY(_start)
SECTIONS
{
    . = 1M;
    _kernel_start = .;
    .text : {
        KEEP(*(.multiboot))
        *(.text)
    }
    .rodata : ALIGN(4096) {
        *(.rodata)
    }
    .data : ALIGN(4096) {
        *(.data)
    }
    .bss : ALIGN(4096) {
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;
}