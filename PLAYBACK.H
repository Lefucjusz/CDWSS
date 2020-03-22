#ifndef _PLAYBACK_H
#define _PLAYBACK_H

#include "inttypes.h"

#define STOPPED 0
#define STARTED 1

#define RESUMED 0
#define PAUSED 1

void playback_start(uint8_t*, uint8_t, uint16_t, uint32_t, uint32_t*, uint16_t);
void playback_continue(uint8_t*, uint8_t, uint16_t, uint32_t*, uint8_t*);
void playback_stop(void);

#endif