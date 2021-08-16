/* timing.h -- Time planner and speech rate control interface
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_TIMING_H
#define RU_TTS_TIMING_H

#include <stdint.h>
#include <stdlib.h>

#include "soundscript.h"


/* Number of time plan rows */
#define TIME_PLAN_ROWS 9

/* Time plan pointer definition */
typedef uint8_t (*time_plan_ptr_t)[100];

typedef struct
{
  uint8_t rate_factor;
  uint8_t stretch;
  uint8_t gaplen;
} timing_t;


/*
 * Build timing draft for specified phonetic transcription.
 *
 * This draft is used for speechrate applying.
 *
 * Returns pointer to the time draft or NULL when failure.
 *
 * This procedure allocates memory, thus, the non-null pointer returned
 * must be finally passed to free().
 */
extern time_plan_ptr_t plan_time(uint8_t *transcription);

/*
 * Apply speechrate parameters to the soundscript
 * according to specified timing data.
 */
extern void apply_speechrate(soundscript_t *script, timing_t *timing, time_plan_ptr_t draft);

#endif
