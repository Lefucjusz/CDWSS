#include "irq.h"
#include "wss.h"
#include <dos.h>

static void interrupt (*old_isr)(void);

void irq_init(void interrupt (*new_isr)(void))
{
	old_isr = getvect(WSS_IRQ + 8); //Store current ISR
	setvect(WSS_IRQ + 8, new_isr); //Hook new ISR
	outp(PIC_MASK_REG, inp(PIC_MASK_REG) & ~(1<<WSS_IRQ)); //Unmask interrupt
}

void irq_release(void)
{
	outp(PIC_MASK_REG, inp(PIC_MASK_REG) | (1<<WSS_IRQ)); //Mask interrupt
	setvect(WSS_IRQ + 8, old_isr); //Restore original ISR
}