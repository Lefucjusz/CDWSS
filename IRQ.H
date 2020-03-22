#ifndef _IRQ_H
#define _IRQ_H

#define PIC_MASK_REG 0x21
#define PIC_STATUS_REG 0x20
#define PIC_END_OF_INTERRUPT 0x20

void irq_init(void interrupt (*)(void));
void irq_release(void);

#endif