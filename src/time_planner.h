/* time_planner.h -- Time planner interface
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_TIME_PLANNER_H
#define RU_TTS_TIME_PLANNER_H

#include <stdint.h>
#include <stdlib.h>

/* Time plan pointer definition */
typedef uint8_t (*time_plan_ptr_t)[100];

/*
 * Fill timing draft for specified phonetic transcription.
 *
 * This draft is used for speechrate applying.
 *
 * Returns non-zero value if the draft is successively filled.
 */
extern int plan_time(uint8_t *transcription, time_plan_ptr_t draft, size_t rows);

#endif
