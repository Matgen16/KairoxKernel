#include "gdt.h"
#include "idt.h"
#include "tty.h"
#include "kprintf.h"
#include "pic.h"
#include "keyboard.h"

void kernel_main(void)
{
    gdt_init();
    idt_init();
    tty_init();

    tty_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    tty_print("OK");

    pic_remap(0x20, 0x28);
    keyboard_init();

    __asm__ volatile ("sti");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}