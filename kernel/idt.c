#include<stdint.h>
#include"idt.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

extern void idt_flush(uint64_t idt_ptr_addr);
extern void idt_stub(void);
extern void irq1_stub(void);

static void idt_set_entry(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr)
{
    idt[vector].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[vector].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[vector].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[vector].selector    = selector;
    idt[vector].ist         = 0;
    idt[vector].type_attr   = type_attr;
    idt[vector].reserved    = 0;
}

void idt_init(void)
{
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint64_t)&idt;

    for (int vector = 0; vector < 256; vector++) {
        idt_set_entry(vector, (uint64_t)idt_stub, 0x08, 0x8E);
    }

    idt_set_entry(0x21, (uint64_t)irq1_stub, 0x08, 0x8E);

    idt_flush((uint64_t)&idtp);
}