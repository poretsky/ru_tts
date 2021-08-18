/* synth.h -- Common speech synthesis related definitions and prototypes
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_SYNTH_H
#define RU_TTS_SYNTH_H

#include <stdlib.h>

#include "transcription.h"
#include "sink.h"
#include "timing.h"
#include "modulation.h"


/* TTS control data structure */
typedef struct
{
  transcription_state_t transcription_state;
  sink_t wave_consumer;
  int flags;

  /* Speechrate parameters */
  timing_t timing;

  /* Pitch and intonation control */
  modulation_t modulation;
} ttscb_t;


/* Function prototypes */

/*
 * Transcription callback function.
 *
 * Finalizes provided transcription and performs speech synthesis for it.
 *
 * The buffer pointed by the first argument should contain a transcription
 * of length specified by the second argument. The last argument
 * should point to the corresponding ttscb structure.
 */
extern int synth_function(void *buffer, size_t length, void *user_data);

#endif
