/* transcription.h -- Phonetic transcription related functions
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_TRANSCRIPTION_H
#define RU_TTS_TRANSCRIPTION_H

#include <stdint.h>

#include "sink.h"


/* General constants */
#define TRANSCRIPTION_START 2
#define TRANSCRIPTION_BUFFER_SIZE 400
#define TRANSCRIPTION_GUARD_SPACE 40
#define TRANSCRIPTION_MAXLEN (TRANSCRIPTION_BUFFER_SIZE - TRANSCRIPTION_GUARD_SPACE)

/* Clause termination flag */
#define CLAUSE_DONE 1


/* Input data holding structure */
typedef struct
{
  char *text;
  char *start;
  char *end;
} input_t;

/* Transcription state control */
typedef struct
{
  uint8_t clause_type;
  uint8_t flags;
} transcription_state_t;


/* Shared data */
extern const char *punctuations;


/* Common functions */

/* Get an item from the list by its number */
extern const uint8_t *list_item(const uint8_t *lst, uint8_t n);

/* Transcribe specified text clause by clause and pass result to the consumer */
extern void process_text(const char *text, sink_t *consumer);

#endif
