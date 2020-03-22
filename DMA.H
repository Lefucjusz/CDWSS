#ifndef _DMA_H
#define _DMA_H

#include "inttypes.h"

void dma_autoinit_init(uint8_t page, uint16_t offset, uint32_t length);
void dma_single_init(uint8_t page, uint16_t offset, uint32_t length);

void dma_restart(void);
void dma_release(void);

#endif