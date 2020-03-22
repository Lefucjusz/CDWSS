#ifndef _BUFFER_H
#define _BUFFER_H

#include "inttypes.h"
#include "cd.h"

#define BUFFER_SIZE_SECTORS 26
#define BUFFER_SIZE_BYTES (CD_SECTOR_SIZE*BUFFER_SIZE_SECTORS)

uint8_t* buffer_alloc(uint8_t*, uint16_t*);
void buffer_load(uint8_t*, uint8_t*, uint32_t*, uint32_t* , uint16_t);
void buffer_free(void);

#endif
