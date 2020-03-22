#include "dma.h"
#include "wss.h"
#include <dos.h>

#define DMAC_MASK_REG 0x0A
#define DMAC_MODE_REG 0x0B
#define DMAC_CLEAR_PTR_REG 0x0C

#define DMAC_MASK_CHANNEL 0x04

#define DMAC_MODE_AUTOINIT_READ 0x58
#define DMAC_MODE_SINGLE_READ 0x48

static const uint8_t dma_page_reg[] = {0x87, 0x83, 0x81, 0x82};

void dma_autoinit_init(uint8_t page, uint16_t offset, uint32_t length) //Program the DMA controller for autoinit transfer
{
	outp(DMAC_MASK_REG, DMAC_MASK_CHANNEL | WSS_DMA); //Mask DMA channel
	outp(DMAC_CLEAR_PTR_REG, 0x00); //Clear byte pointer
	outp(DMAC_MODE_REG, DMAC_MODE_AUTOINIT_READ | WSS_DMA); //Set autoinit mode
	outp(WSS_DMA << 1, offset & 0xFF); //Write the offset
	outp(WSS_DMA << 1, offset >> 8);
	outp((WSS_DMA << 1) + 1, (length - 1) & 0xFF); //Set the block length
	outp((WSS_DMA << 1) + 1, ((length - 1) >> 8) & 0xFF);
	outp(dma_page_reg[WSS_DMA], page); //Write the page
	outp(DMAC_MASK_REG, WSS_DMA); //Unmask DMA channel
}

void dma_single_init(uint8_t page, uint16_t offset, uint32_t length) //Program the DMA controller for single cycle transfer
{
	outp(DMAC_MASK_REG, DMAC_MASK_CHANNEL | WSS_DMA); //Mask DMA channel
	outp(DMAC_CLEAR_PTR_REG, 0x00); //Clear byte pointer
	outp(DMAC_MODE_REG, DMAC_MODE_SINGLE_READ | WSS_DMA); //Set autoinit mode
	outp(WSS_DMA << 1, offset & 0xFF); //Write the offset
	outp(WSS_DMA << 1, offset >> 8);
	outp((WSS_DMA << 1) + 1, (length - 1) & 0xFF); //Set the block length
	outp((WSS_DMA << 1) + 1, ((length - 1) >> 8) & 0xFF);
	outp(dma_page_reg[WSS_DMA], page); //Write the page
	outp(DMAC_MASK_REG, WSS_DMA); //Unmask DMA channel
}

void dma_release(void)
{
	outp(DMAC_MASK_REG, DMAC_MASK_CHANNEL | WSS_DMA); //Mask DMA channel
}