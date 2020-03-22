#include "buffer.h"
#include "cd.h"
#include <alloc.h>
#include <dos.h>

static uint8_t* buf;

uint8_t* buffer_alloc(uint8_t* page, uint16_t* off) //Allocates, zero-fills and alignes buffer, return pointer to aligned space, space page and offset passed as arguments
{
	uint8_t* buf_aligned;
	uint32_t lma;

	/* Allocation */
	buf = farmalloc(BUFFER_SIZE_BYTES*2); //Allocate twice the buffer size

	if(!buf) //If unsuccessful...
	{
		return 0; //...return null pointer
	}

	/* Page alignment */
	lma = ((uint32_t)FP_SEG(buf) << 4) + FP_OFF(buf); //Compute logical address
	(*page) = lma >> 16; //Compute page of the buffer beginning
	(*page)++; //From the beginning of the next page we have at least BUFFER_SIZE memory on one page

	(*off) = 0; //Offset is zero

	buf_aligned = MK_FP((*page)*0x1000, *off); //Make pointer to the aligned buffer

	//buffer_memset(buf_aligned, 0, BUFFER_SIZE_BYTES);

	return buf_aligned; //Return pointer to the aligned space
}

void buffer_load(uint8_t* buf_aligned, uint8_t* buf_half, uint32_t* starting_sector, uint32_t* sectors_to_read, uint16_t device)
{
	if((*sectors_to_read) == 0) //If nothing to read...
	{
		return; //...return
	}

	if((*sectors_to_read) >= BUFFER_SIZE_SECTORS/2) //If more than half of the buffer to be filled
	{
		if(*buf_half)
		{
			cd_read_long(device, (*starting_sector), BUFFER_SIZE_SECTORS/2, buf_aligned+(BUFFER_SIZE_BYTES/2)); //Load full upper half
		}
		else
		{
			cd_read_long(device, (*starting_sector), BUFFER_SIZE_SECTORS/2, buf_aligned); //Load full lower half
		}
		(*sectors_to_read) -= BUFFER_SIZE_SECTORS/2; //Next half of the buffer loaded
		(*starting_sector) += BUFFER_SIZE_SECTORS/2; //Move starting sector pointer
	}
	else
	{
		if(*buf_half)
		{
			memset(buf_aligned+(BUFFER_SIZE_BYTES/2), 0, BUFFER_SIZE_BYTES/2); //Zero-fill upper half to avoid glitches
			cd_read_long(device, (*starting_sector), (*sectors_to_read), buf_aligned+(BUFFER_SIZE_BYTES/2)); //Load all the remaining sectors to upper half
		}
		else
		{
			memset(buf_aligned, 0, BUFFER_SIZE_BYTES/2); //Zero-fill lower half to avoid glitches
			cd_read_long(device, (*starting_sector), (*sectors_to_read), buf_aligned); //Load all the remaining sectors to lower half
		}
		(*sectors_to_read) = 0; //Everything has been loaded
		//(*starting_sector) += (*sectors_to_read); //This is not needed
	}
	(*buf_half) ^= 1; //Change buffer half to be filled
}

void buffer_free(void)
{
	farfree(buf); //Free all space allocated
}
