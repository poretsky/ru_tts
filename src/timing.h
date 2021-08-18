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

/* Number of punctuations treated as clause separators */
#define CLAUSE_SEPARATORS 7


/* Time plan pointer definition */
typedef uint8_t (*time_plan_ptr_t)[100];

typedef struct
{
  uint16_t rate_factor;
  uint8_t gap_factor;
  uint8_t gaplen[CLAUSE_SEPARATORS];
} timing_t;


/*
 * Initial timing setup for specified speech rate and relative
 * interclause gap duration expressed as a percentage
 * of the default value.
 */
extern void timing_setup(timing_t *timing, int speech_rate, int gap_factor);

/*
 * Adjust gap duration for specified separator
 * applying a percentage factor.
 */
extern void adjust_gaplen(timing_t *timing, char separator, int gap_factor);

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
