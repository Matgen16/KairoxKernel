#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* Remaps the two 8259 PICs so IRQs 0-15 fire on interrupt vectors
 * offset1..offset1+7 (master) and offset2..offset2+7 (slave), instead
 * of colliding with the CPU's own exception vectors 0-31. */
void pic_remap(uint8_t offset1, uint8_t offset2);

/* Unmasks (enables) a single IRQ line, 0-15. */
void pic_unmask_irq(uint8_t irq);

/* Sends End-Of-Interrupt to the PIC(s) after handling an IRQ. */
void pic_send_eoi(uint8_t irq);

#endif
