/* soundscript.h -- Sound script definition
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_SOUNDSCRIPT_H
#define RU_TTS_SOUNDSCRIPT_H

#include <stdint.h>
#include <stdlib.h>

#include "voice.h"


/* Constant values */
#define MAX_SOUNDS 1000
#define NSTAGES 12


/* Sound unit structure */
typedef struct
{
  uint8_t id;
  uint8_t stage;
  uint16_t duration;
} sound_unit_t;

/* Intonation control parameters */
typedef struct
{
  uint8_t stretch;
  int8_t delta;
  uint8_t count;
  uint8_t period;
} icb_t;

/* Sound mastering script */
typedef struct
{
  const voice_t *voice;
  size_t length;
  sound_unit_t sounds[MAX_SOUNDS];
  icb_t icb[NSTAGES];
} soundscript_t;

#endif
