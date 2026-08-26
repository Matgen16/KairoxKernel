#include <stdarg.h>
#include <stddef.h>
#include "tty.h"
#include "kprintf.h"

static void print_uint(unsigned long value, unsigned base, int uppercase) {
    char buf[32];
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;

    if (value == 0) {
        tty_putchar('0');
        return;
    }

    while (value > 0) {
        buf[i++] = digits[value % base];
        value /= base;
    }
    while (i > 0) {
        tty_putchar(buf[--i]);
    }
}

static void print_int(long value) {
    if (value < 0) {
        tty_putchar('-');
        print_uint((unsigned long)(-value), 10, 0);
    } else {
        print_uint((unsigned long)value, 10, 0);
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            tty_putchar(fmt[i]);
            continue;
        }

        i++;
        switch (fmt[i]) {
            case 'd':
                print_int(va_arg(args, int));
                break;
            case 'u':
                print_uint(va_arg(args, unsigned int), 10, 0);
                break;
            case 'x':
                print_uint(va_arg(args, unsigned int), 16, 0);
                break;
            case 'X':
                print_uint(va_arg(args, unsigned int), 16, 1);
                break;
            case 's':
                tty_print(va_arg(args, const char *));
                break;
            case 'c':
                tty_putchar((char)va_arg(args, int));
                break;
            case '%':
                tty_putchar('%');
                break;
            case '\0':
                va_end(args);
                return;
            default:
                tty_putchar('%');
                tty_putchar(fmt[i]);
                break;
        }
    }

    va_end(args);
}
