#include <dos.h>
/*
* 	Play Audio CD digitally on Windows Sound System compatible soundcard
*	(C) Lefucjusz, Gdansk 03.2020
*
* 	This program has to be compiled with compact memory model
*
* 	Things to develop:
*
*	-repair pausing - after pause that's long enough for drive to spin down it sometimes glitches
*	-graphic interface
*	-add WSS detection, now it's hardcoded base=0x530,irq=5,dma=1
*
*
*/
#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "inttypes.h"
#include "cd.h"
#include "buffer.h"
#include "irq.h"
#include "wss.h"
#include "playback.h"

#define KEY_ESC 27

volatile uint8_t irq_fired = 0;

void interrupt irq_isr(void)
{
	irq_fired = 1;
	outp(WSS_STATUS_REG, 0x00); //Clear WSS interrupt bit
	outp(PIC_STATUS_REG, PIC_END_OF_INTERRUPT); //Acknowledge interrupt
}

int main(void)
{
	uint32_t *tracks_locations, *tracks_lengths;
	uint32_t sectors_qty, sector = 0;
	uint16_t drives_qty, first_drive, mscdex_ver, selected_drive, offset, i;
	uint8_t *drive_letters, *audio_buffer;
	uint8_t tracks_qty, hr, min, min_played, sec, sec_played, page, play_state = STOPPED, pause_state = RESUMED, track_played = 0;
	char input;

	clrscr(); //Clear display
	printf("Digital CD Audio player for WSS-compatible card\n(c)Copyright Lefucjusz, 2020\n\n");
	
	/* Check if any drives installed */
	cd_check_drives(&drives_qty, &first_drive, &mscdex_ver); //Check if any drive is available
	
	if(drives_qty == 0) //If no drive
	{
		printf("MSCDEX not installed! Terminating...\n");
		return -1;
	}
	
	printf("MSCDEX detected, version %u.%u\n", (mscdex_ver >> 8), (mscdex_ver & 0xFF));
	
	/* Drive selection */
	if(drives_qty == 1) //If only one drive
	{
		printf("Found 1 drive: %c\n", first_drive + 'A');
		selected_drive = first_drive;
	}
	else //If more than one drive; this case has never been tested as I couldn't connect more than one drive to my test computer
	{
		drive_letters = farmalloc(sizeof(uint8_t)*drives_qty); //Allocate memory for letters array
		if(!drive_letters) //If failed to allocate
		{
			printf("Failed to allocate memory for drive letters! Terminating...\n");
			return -2;
		}
		
		cd_get_drive_letters(drive_letters); //Get drive letters
		
		printf("Found %u drives: ", drives_qty);
		for(i = 0; i < drives_qty; i++)
		{
			printf("%c ", drive_letters[i]+'A');
		}
		
		printf("Select drive for playback: ");
		input = getch();
		for(i = 0; i < drives_qty; i++) //Check if such drive is available
		{
			if((drive_letters[i] + 'A') == toupper(input))
			{
				selected_drive = drive_letters[i];
				farfree(drive_letters); //Free letters array
				break;
			}
		}
		if(i == drives_qty) //Nothing has been found
		{
			printf("No such drive available! Terminating...\n");
			farfree(drive_letters); //Free letters array
			return -3;		
		}
	}	
	printf("Selected drive: %c\n\n", selected_drive + 'A');
	
	cd_init(selected_drive); //Initialize CD driver
	
	/* Read TOC */
	printf("Reading disc information...\n");	
	sleep(2); //Time for messages to display
	while(!cd_check_if_audiocd(selected_drive)) //Loop until proper CD inserted or quit on escape key
	{
		printf("Drive not ready or inserted CD is not an Audio CD\nPress any key to retry, ESC to quit...\n");
		input = getch();
		if(input == KEY_ESC)
		{
			return -4;
		}
	}
	
	cd_get_disc_info(selected_drive, &tracks_qty, &sectors_qty); //Obtain disc info
	
	tracks_locations = farmalloc(sizeof(uint32_t)*tracks_qty);
	tracks_lengths = farmalloc(sizeof(uint32_t)*tracks_qty);
	if(!tracks_locations || !tracks_lengths) //Potential memory leak?
	{
		printf("Failed to allocate memory for TOC data! Terminating...\n");
		return -5;
	}
	
	for(i = 0; i < tracks_qty; i++) //Obtain tracks locations
	{
		tracks_locations[i] = cd_get_track_location(selected_drive, i+1);
	}

	for(i = 0; i < tracks_qty-1; i++) //Compute tracks lengths
	{
		tracks_lengths[i] = tracks_locations[i+1] - tracks_locations[i];
	}
	tracks_lengths[tracks_qty-1] = sectors_qty - tracks_locations[tracks_qty-1]; //Last one has to be computed differently
	
	clrscr(); //Clear display
	printf("---------------------\t\tControl:\n");
	printf("------ CD Info ------\t\t[ESC] - leave to DOS\n");
	printf("---------------------\t\t[I] - previous track\n");
	printf("| #Track |  Length  |\t\t[O] - next track\n");
	printf("|--------|----------|\t\t[P] - pause/resume\n");
	for(i = 0; i < tracks_qty; i++)
	{
		cd_sectors_to_time(tracks_lengths[i], &hr, &min, &sec);
		printf("|   %02u   | %02u:%02u:%02u |\n", i+1, hr, min, sec);
	}
	cd_sectors_to_time(sectors_qty, &hr, &min, &sec);
	printf("---------------------\n");
	printf("|  Full  | %02u:%02u:%02u |\n", hr, min, sec);
	
	/* Start playback */	
	audio_buffer = buffer_alloc(&page, &offset);
	if(!audio_buffer)
	{
		printf("Failed to allocate memory for playback buffer! Terminating...\n");
		farfree(tracks_lengths);
		farfree(tracks_locations);
		return -6;
	}	
	
	cd_lock(selected_drive); //Lock CD drive
	irq_init(irq_isr); //Program PIC
	playback_start(audio_buffer, page, offset, sectors_qty, &sector, selected_drive); //Start playback
	play_state = STARTED; //Set play state flag
	
	while(input != KEY_ESC && (play_state == STARTED)) //Loop until CD is playing or ESC is pressed
	{
		if(kbhit()) //If keystroke
		{
			input = getch(); //Get that keystroke
			switch(toupper(input)) //toupper for being independent of input case
			{
				case 'I': //Previous track
					if((track_played > 0) && (pause_state == RESUMED)) //If track is not first and not paused (if paused it would start and infinitely loop first 64KB of file...)
					{
						track_played--; //Decrement track pointer
						playback_stop(); //Stop playback
						sector = tracks_locations[track_played]; //Store the position of track beginning
						playback_start(audio_buffer, page, offset, (sectors_qty-tracks_locations[track_played]), &sector, selected_drive); //Restart playback there
					}
				break;
				
				case 'O': //Next track
					if((track_played < tracks_qty-1) && (pause_state == RESUMED))  //If track is not last and not paused (if paused it would start and infinitely loop first 64KB of file...)
					{
						track_played++; //Increment track pointer
						playback_stop(); //Stop playback
						sector = tracks_locations[track_played]; //Store the position of track beginning
						playback_start(audio_buffer, page, offset, (sectors_qty-tracks_locations[track_played]), &sector, selected_drive); //Restart playback there
					}
				break;

				case 'P':
					if(pause_state == PAUSED) //If paused
					{
						pause_state = RESUMED; //Resume
						playback_resume();
					}
					else //If resumed
					{
						pause_state = PAUSED; //Pause
						playback_stop();
					}
				break;
				
				default: //Do nothing
				break;
			}
		}
		if(irq_fired && (pause_state == RESUMED)) //If IRQ has been received and playback is not paused
		{
			irq_fired = 0;
			playback_continue(audio_buffer, page, offset, &sector, &play_state); //Continue playback (load another portion of data to buffer)

			if(sector >= tracks_locations[track_played+1]) //If next sector to be read is a part of next track
			{
					track_played++; //Increment track pointer
			}
			cd_sectors_to_time(sector-tracks_locations[track_played], &hr, &min_played, &sec_played); //Recalculate track playback time (hr is discarded)
			cd_sectors_to_time(tracks_lengths[track_played], &hr, &min, &sec); //Recalculate track length
			printf("\r\t\t\t\tPlaying track: #%u, %02u:%02u/%02u:%02u ", track_played+1, min_played, sec_played, min, sec);
		}
	}
	
	playback_stop(); //Stop playback
	cd_unlock(selected_drive); //Unlock CD tray
	cd_deinit(selected_drive); //Deinit CD driver
	irq_release(); //Mask WSS interrupt
	buffer_free(); //Free memory allocated for playback buffer
	farfree(tracks_lengths); //Free memory allocated for TOC
	farfree(tracks_locations);
	return 0;
}
