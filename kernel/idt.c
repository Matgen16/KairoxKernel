#include <stdint.h>
#include "idt.h"
#include "isr.h"
#include "pit.h"
#include "kprintf.h"

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
extern void irq0_stub(void);
extern void irq1_stub(void);

// 1. Declare Assembly ISR stubs
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);  extern void isr3(void);
extern void isr4(void);  extern void isr5(void);  extern void isr6(void);  extern void isr7(void);
extern void isr8(void);  extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

static void (*isr_stub_table[32])(void) = {
    isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
    isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

void idt_set_gate(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr)
{
    idt[vector].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[vector].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[vector].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[vector].selector    = selector;
    idt[vector].ist         = 0;
    idt[vector].type_attr   = type_attr;
    idt[vector].reserved    = 0;
}

// 2. Define isr_install (loads all 32 exception gates into IDT)
void isr_install(void)
{
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint64_t)isr_stub_table[i], 0x08, 0x8E);
    }
}

// 3. Define C handler called by isr_common in isr.asm
void isr_handler(isr_frame_t *frame)
{
    kprintf("CPU Exception %d | Error Code: 0x%x | RIP: 0x%x\n", 
            frame->int_no, frame->err_code, frame->rip);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void idt_init(void)
{
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint64_t)&idt;

    for (int vector = 0; vector < 256; vector++) {
        idt_set_gate(vector, (uint64_t)idt_stub, 0x08, 0x8E);
    }

    isr_install();

    idt_set_gate(0x20, (uint64_t)irq0_stub, 0x08, 0x8E);
    idt_set_gate(0x21, (uint64_t)irq1_stub, 0x08, 0x8E);

    idt_flush((uint64_t)&idtp);
}