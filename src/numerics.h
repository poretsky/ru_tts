/* numerics.h -- Numeric string transcription
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_NUMERICS_H
#define RU_TTS_NUMERICS_H

#include <stdint.h>
#include <stdlib.h>

#include "transcription.h"
#include "sink.h"


/* Digit detection macro */
#define IS_DIGIT(c) ((c >= '0') && (c <= '9'))


/* Global functions */

/* Transcribe numeric string from input and pass result to the consumer */
extern void process_number(input_t *input, sink_t *consumer);

#endif
