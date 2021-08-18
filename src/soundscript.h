/* soundscript.h -- Sound script definition and usage
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
#include "sink.h"
#include "timing.h"
#include "modulation.h"


/* Data size definition */
#define MAX_SOUNDS 1000
#define NSTAGES 12

/* Number of time plan rows */
#define TIME_PLAN_ROWS 9

/* Voice sample length threshold for processing differentiation */
#define VOICE_THRESHOLD 105


/* Time plan pointer definition */
typedef uint8_t (*time_plan_ptr_t)[100];

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


/* Available voices */
extern const voice_t male;
extern const voice_t female;


/* Related functions */

/* Build utterance according to phonetic transcription */
extern void build_utterance(uint8_t *transcription, soundscript_t *script);

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

/* Apply intonation parameters to the sound script */
extern void apply_intonation(uint8_t *transcription, soundscript_t *soundscript,
                             modulation_t *modulation, uint8_t clause_type);

/* Generate sound stream and feed it to the specified sink */
extern void make_sound(soundscript_t *script, sink_t *consumer);

#endif
