/* voice.h -- Voice data definition
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_VOICE_H
#define RU_TTS_VOICE_H

#include <stdint.h>


/* Constant values */
#define VOICE_THRESHOLD 105
#define VOICE_DIMENSION 201


/* Voice data structure */
typedef struct
{
  uint16_t sound_offsets[VOICE_DIMENSION]; /* voice_data1 */
  uint16_t sound_lengths[VOICE_DIMENSION]; /* voice_data2 */
  int8_t samples[]; /* voice_data3 */
} voice_t;

#endif
