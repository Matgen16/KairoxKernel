#include "tty.h"
#include "io.h"

static volatile uint16_t *const vga = VGA_BUFFER;
static size_t cursor_row = 0;
static size_t cursor_col = 0;
static uint8_t current_color = VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4);

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

void tty_set_color(enum vga_color fg, enum vga_color bg) {
    current_color = (uint8_t)fg | ((uint8_t)bg << 4);
}

void tty_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void tty_update_cursor(void) {
    uint16_t pos = (uint16_t)(cursor_row * VGA_COLS + cursor_col);

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void tty_init(void) {
    cursor_row = 0;
    cursor_col = 0;
    for (size_t row = 0; row < VGA_ROWS; row++) {
        for (size_t col = 0; col < VGA_COLS; col++) {
            vga[row * VGA_COLS + col] = vga_entry(' ', current_color);
        }
    }
    tty_enable_cursor(14, 15);
    tty_update_cursor();
}

void tty_putchar(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = VGA_COLS - 1;
        }
        vga[cursor_row * VGA_COLS + cursor_col] = vga_entry(' ', current_color);
    } else {
        vga[cursor_row * VGA_COLS + cursor_col] = vga_entry(c, current_color);
        cursor_col++;
        if (cursor_col >= VGA_COLS) {
            cursor_col = 0;
            cursor_row++;
        }
    }

    if (cursor_row >= VGA_ROWS) {
        tty_scroll();
    }

    tty_update_cursor();
}

void tty_print(const char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        tty_putchar(str[i]);
    }
}

void tty_scroll(void) {
    for (size_t row = 1; row < VGA_ROWS; row++) {
        for (size_t col = 0; col < VGA_COLS; col++) {
            vga[(row - 1) * VGA_COLS + col] = vga[row * VGA_COLS + col];
        }
    }
    for (size_t col = 0; col < VGA_COLS; col++) {
        vga[(VGA_ROWS - 1) * VGA_COLS + col] = vga_entry(' ', current_color);
    }
    cursor_row = VGA_ROWS - 1;
}