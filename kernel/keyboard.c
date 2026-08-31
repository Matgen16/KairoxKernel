#include <stdint.h>
#include "io.h"
#include "tty.h"
#include "pic.h"
#include "keyboard.h"

#define KBD_DATA_PORT    0x60
#define KBD_STATUS_PORT  0x64
#define KBD_CMD_PORT     0x64

#define KBD_STATUS_OUTPUT 0x01
#define KBD_STATUS_INPUT  0x02

#define KBD_CMD_ENABLE  0xAE

#define IRQ_KEYBOARD  1

#define SC_LSHIFT      0x2A
#define SC_RSHIFT      0x36
#define SC_LSHIFT_REL  0xAA
#define SC_RSHIFT_REL  0xB6

static int shift_held = 0;

static const char scancode_ascii[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' ', 0,
};

static const char scancode_ascii_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,   ' ', 0,
};

void keyboard_drain(void) {
    while (inb(KBD_STATUS_PORT) & KBD_STATUS_OUTPUT) {
        (void)inb(KBD_DATA_PORT);
    }
}

void keyboard_init(void) {
    while (inb(KBD_STATUS_PORT) & KBD_STATUS_INPUT) {
        io_wait();
    }

    outb(KBD_CMD_PORT, KBD_CMD_ENABLE);
    io_wait();

    keyboard_drain();
    pic_unmask_irq(IRQ_KEYBOARD);
}

void keyboard_handler(void) {
    if (!(inb(KBD_STATUS_PORT) & KBD_STATUS_OUTPUT)) {
        pic_send_eoi(IRQ_KEYBOARD);
        return;
    }

    uint8_t scancode = inb(KBD_DATA_PORT);

    switch (scancode) {
        case SC_LSHIFT:
        case SC_RSHIFT:
            shift_held = 1;
            break;
        case SC_LSHIFT_REL:
        case SC_RSHIFT_REL:
            shift_held = 0;
            break;
        default:
            if (!(scancode & 0x80)) {
                char c = shift_held ? scancode_ascii_shift[scancode]
                                     : scancode_ascii[scancode];
                if (c) {
                    tty_putchar(c);
                }
            }
            break;
    }

    pic_send_eoi(IRQ_KEYBOARD);
}
