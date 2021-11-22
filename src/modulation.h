/* modulation.h -- Voice modulation control means
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_MODULATION_H
#define RU_TTS_MODULATION_H

#include <stdint.h>

/* Pitch and intonation control data */
typedef struct
{
  uint16_t mintone;
  uint16_t maxtone;
} modulation_t;

/*
 * Setup modulation parameters according to specified voice pitch
 * and intonation level expressed as a percentage of the default values.
 */
extern void modulation_setup(modulation_t *modulation, int voice_pitch, int intonation);

#endif
