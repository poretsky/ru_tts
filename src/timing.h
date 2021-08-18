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


/* Number of punctuations treated as clause separators */
#define CLAUSE_SEPARATORS 7


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

#endif
