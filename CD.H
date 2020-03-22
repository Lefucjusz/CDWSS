#ifndef _CD_H
#define _CD_H

#include "inttypes.h"

#define CD_SECTOR_SIZE 2352L //bytes
#define CD_SECOND_SIZE 176400L //bytes

void cd_check_drives(uint16_t*, uint16_t*, uint16_t*);
void cd_get_drive_letters(uint8_t*);

uint16_t cd_init(uint16_t);
uint16_t cd_deinit(uint16_t);

void cd_open(uint16_t);
void cd_close(uint16_t);

void cd_lock(uint16_t);
void cd_unlock(uint16_t);

uint8_t cd_check_if_audiocd(uint16_t);

void cd_read_long(uint16_t, uint32_t, uint32_t, void*);
void cd_get_disc_info(uint16_t, uint8_t*, uint32_t*);
uint32_t cd_get_track_location(uint16_t, uint8_t);

void cd_sectors_to_time(uint32_t sectors, uint8_t* hours, uint8_t* minutes, uint8_t* seconds);

#endif