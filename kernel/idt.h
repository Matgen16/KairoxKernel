#ifndef IDT_H
#define IDT_H

#include <stdint.h>

void idt_init(void);

void idt_set_gate(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr);

#endif