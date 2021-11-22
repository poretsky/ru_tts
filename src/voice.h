/* voice.h -- Voice data definition
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_VOICE_H
#define RU_TTS_VOICE_H

#include <stdint.h>


/* Number of voice samples */
#define VOICE_DIMENSION 201


/* Voice data structure */
typedef struct
{
  unsigned int pitch_factor;
  uint16_t sound_offsets[VOICE_DIMENSION];
  uint16_t sound_lengths[VOICE_DIMENSION];
  int8_t samples[];
} voice_t;

#endif
