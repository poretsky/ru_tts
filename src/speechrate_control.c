/* speechrate_control.c -- Apply speechrate parameters to a sound script
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "soundscript.h"
#include "timing.h"
#include "transcription.h"


/* Local static data */

/* Bottom duration threshold for all voice sounds */
static const uint8_t bottom[] =
  {
    30, 31, 32, 31, 30, 30,
    31, 32, 31, 30, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 30, 30, 30, 30,
    30, 15, 15, 15, 15, 15,
    30, 30, 30, 30, 30, 15,
    15, 15, 15, 15, 30, 30,
    30, 30, 30, 15, 15, 15,
    15, 15, 30, 30, 30, 30,
    30, 15, 15, 15, 15, 15,
    30, 30, 30, 30, 30, 10,
    11, 12, 11, 10, 10, 11,
    12, 11, 10, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 20, 30, 35, 20,
    40, 20, 35, 49, 30, 45,
    20, 40, 50, 40, 50, 60,
    50, 0, 20, 25, 10, 35,
    35, 20, 20, 30, 20, 35,
    40, 0, 20, 25, 0, 20,
    25, 20, 20, 30, 20, 20,
    30, 50, 50, 40, 40, 35,
    35, 50, 40, 40, 50, 30,
    30, 30, 30, 40, 50, 60,
    60, 80, 70, 70, 90, 50,
    60, 50, 50, 5, 0, 255,
    255, 0, 255, 255, 255, 255,
    255, 255, 255
  };

/* Top duration threshold for all voice sounds */
static const uint8_t top[] =
  {
    75, 78, 80, 78, 75, 55,
    58, 60, 58, 55, 30, 30,
    30, 30, 30, 25, 25, 25,
    25, 25, 30, 30, 30, 30,
    30, 25, 25, 25, 25, 25,
    30, 30, 30, 30, 30, 25,
    25, 25, 25, 25, 30, 30,
    30, 30, 30, 25, 25, 25,
    25, 25, 60, 60, 60, 60,
    60, 25, 25, 25, 25, 25,
    50, 50, 50, 50, 50, 25,
    25, 25, 25, 25, 50, 50,
    50, 50, 50, 25, 25, 25,
    25, 25, 50, 50, 50, 50,
    50, 25, 25, 25, 25, 25,
    60, 60, 60, 60, 60, 50,
    50, 50, 50, 50, 50, 50,
    50, 50, 50, 30, 30, 35,
    30, 35, 30, 30, 30, 30,
    30, 35, 30, 30, 30, 30,
    35, 30, 40, 80, 90, 45,
    110, 50, 90, 100, 50, 110,
    40, 100, 110, 100, 100,
    110, 100, 20, 30, 30, 23,
    80, 40, 30, 30, 50, 35,
    50, 60, 19, 30, 30, 19,
    30, 30, 30, 30, 50, 30,
    30, 50, 110, 100, 80, 100,
    80, 70, 130, 120, 110, 130,
    90, 90, 80, 80, 90, 100,
    150, 150, 160, 170, 170,
    210, 130, 150, 130, 130,
    50, 0, 20, 30, 25, 25,
    40, 50, 15, 255, 55, 255
  };

/* Sound duration forming elements */
static const uint8_t elements[][6] =
  {
    { 20, 20, 0, 0, 0, 0 },
    { 50, 25, 25, 0, 0, 0 },
    { 10, 2, 1, 0, 0, 0 },
    { 20, 10, 0, 0, 0, 0 },
    { 60, 30, 0, 0, 0, 0 },
    { 30, 15, 0, 0, 0, 0 },
    { 50, 40, 20, 0, 0, 0 },
    { 20, 12, 6, 0, 0, 0 },
    { 0, 8, 12, 40, 0, 0 }
  };


/* Global functions */

/*
 * Initial timing setup for specified speech rate and relative
 * interclause gap duration expressed as a percentage
 * of the default value.
 */
void timing_setup(timing_t *timing, int speech_rate, int gap_factor)
{
  int stretch, i;

  if (speech_rate < 40)
    stretch = 500;
  else if (speech_rate > 250)
    stretch = 80;
  else stretch = 20000 / speech_rate;
  timing->rate_factor = stretch - 80;
  timing->gap_factor = (uint8_t) (stretch * (gap_factor << 1) / 500);
  for (i = 0; i < CLAUSE_SEPARATORS; i++)
    timing->gaplen[i] = top[i + 191];
}

/*
 * Adjust gap duration for specified separator
 * applying a percentage factor.
 */
void adjust_gaplen(timing_t *timing, char separator, int gap_factor)
{
  char *s = memchr(punctuations, separator, CLAUSE_SEPARATORS);
  if (s)
    {
      int i = s - punctuations;
      int gaplen = top[i + 191] * gap_factor / 100;
      if (gaplen < 0)
        timing->gaplen[i] = 0;
      else if (gaplen > 150)
        timing->gaplen[i] = 150;
      else timing->gaplen[i] = (uint8_t) gaplen;
    }
}

/*
 * Apply speechrate parameters to the soundscript
 * according to specified timing data.
 */
void apply_speechrate(soundscript_t *script, timing_t *timing, time_plan_ptr_t draft)
{
  uint16_t i;
  uint8_t n = 1;

  for (i = 0; i < script->length; i++)
    {
      uint8_t j = script->sounds[i].id;
      if (j < 189)
        {
          if ((draft[1][n] != 4) || (script->sounds[i].stage != 3))
            {
              uint32_t s = 0;
              uint8_t k;
              for (k = 0; k < TIME_PLAN_ROWS; k++)
                s += elements[k][draft[k][n]];
              s *= top[j] - bottom[j];
              s *= timing->rate_factor;
              s += (((uint32_t) bottom[j]) << 14) + 2048;
              s >>= 12;
              if ((draft[1][n] == 5) && (script->sounds[i].stage == 2))
                s += s >> 1;
              script->sounds[i].duration = (uint16_t)s;
            }
          else script->sounds[i].duration = 0;
          if (script->sounds[i].stage >= script->sounds[i + 1].stage)
            if (draft[1][n++] == 5)
              {
                while ((++i) < script->length)
                  {
                    script->sounds[i].duration = 0;
                    if (script->sounds[i].stage >= script->sounds[i + 1].stage)
                      break;
                  }
                n++;
              }
        }
      else
        {
          int k = j - 191;
          uint8_t gaplen = ((k >= 0) && (k < CLAUSE_SEPARATORS)) ? timing->gaplen[k] : top[j];
          script->sounds[i].duration = ((uint16_t)(timing->gap_factor)) * ((uint16_t)gaplen);
        }
    }
}
