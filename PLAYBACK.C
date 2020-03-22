#include "playback.h"
#include "buffer.h"
#include "wss.h"

static uint32_t sectors_to_read, sectors_to_play;
static uint16_t drive;
static uint8_t buffer_half;

void playback_start(uint8_t* buffer, uint8_t page, uint16_t offset, uint32_t sectors_qty, uint32_t* sector, uint16_t device) //Sets local playback variables, starts playback
{
	buffer_half = 0; //Set buffer to first half
	sectors_to_read = sectors_qty; //Store local variables
	sectors_to_play = sectors_qty;
	drive = device;
	buffer_load(buffer, &buffer_half, sector, &sectors_to_read, drive); //Load lower half of buffer
	buffer_load(buffer, &buffer_half, sector, &sectors_to_read, drive); //Load upper half of buffer
	dma_autoinit_init(page, offset, BUFFER_SIZE_BYTES); //Program DMA controller for autoinit mode
	wss_start(); //Initialize soundcard
}

void playback_continue(uint8_t* buffer, uint8_t page, uint16_t offset, uint32_t* sector, uint8_t* playback_state) //Called every IRQ, reloads buffer with fresh data, updates playback variables
{
	buffer_load(buffer, &buffer_half, sector, &sectors_to_read, drive); //Load proper half of the buffer
	if(sectors_to_play > 0) //If there's still something to play
	{
		if(sectors_to_play > BUFFER_SIZE_SECTORS/2) //If there is still more than half of the buffer to play
		{
			sectors_to_play -= BUFFER_SIZE_SECTORS/2; //Half of the buffer has just been played
		}
		
		if(sectors_to_play <= BUFFER_SIZE_SECTORS/2) //If after the previous play there is less or exactly half of the buffer to play now
		{
			/* 
			 * Play the last samples - offset is set so that it points to the most recently loaded buffer half;
			 * Whole half of the buffer can be played - it's zero-filled anyways
			 */
			dma_single_init(page, offset+(BUFFER_SIZE_BYTES/2)*(buffer_half?0:1), BUFFER_SIZE_BYTES/2); //Program DMA controller for single cycle mode
			sectors_to_play = 0; //Everything has been played
		}
	}
	else //Otherwise
	{
		(*playback_state) = STOPPED; //Stop playback
	}
}

void playback_resume(void)
{
	wss_restart();
}

void playback_stop(void)
{
	wss_stop();
}