/* synth.c -- Phonetic transcription to speech transformation
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

#include "phonemes.h"
#include "synth.h"
#include "soundscript.h"
#include "transcription.h"
#include "ru_tts.h"


/* Local data */

static const uint8_t seqlist1[] =
  {
    1, PH_A,
    5, PH_V, PH_R, PH_O, PH_D_, PH_E,
    4, PH_D, PH_A, PH_G_, PH_E,
    2, PH_D, PH_A,
    5, PH_J, PH_E, PH_D, PH_V, PH_A,
    5, PH_J, PH_E, PH_S, PH_L_, PH_I,
    3, PH_I, PH_L_, PH_I,
    4, PH_I, PH_T, PH_A, PH_K,
    5, PH_I, PH_N, PH_A, PH_CH, PH_E,
    1, PH_I,
    3, PH_K, PH_A, PH_K,
    4, PH_L_, PH_I, PH_B, PH_O,
    2, PH_N, PH_O,
    3, PH_R, PH_A, PH_S,
    5, PH_T, PH_A, PH_K, PH_G_, PH_E,
    3, PH_T, PH_A, PH_K,
    3, PH_T_, PH_E, PH_M,
    4, PH_T, PH_O, PH_G_, PH_E,
    3, PH_X, PH_O, PH_T_,
    3, PH_CH, PH_E, PH_M,
    5, PH_CH, PH_T, PH_O, PH_B, PH_Y,
    4, PH_CH, PH_T, PH_O, PH_P,
    3, PH_CH, PH_T, PH_O,
    4, PH_I, PH_T, PH_A, PH_G,
    3, PH_K, PH_A, PH_G,
    3, PH_R, PH_A, PH_Z,
    3, PH_T, PH_A, PH_G,
    3, PH_X, PH_O, PH_D_,
    4, PH_CH, PH_T, PH_O, PH_B,
    2, PH_T, PH_O,
    0
  };

static const uint8_t seqlist2[] =
  {
    3, PH_B_, PH_E, PH_S,
    3, PH_B_, PH_E, PH_Z,
    4, PH_V, PH_D, PH_O, PH_L_,
    4, PH_B, PH_L_, PH_I, PH_S,
    4, PH_B, PH_L_, PH_I, PH_Z,
    5, PH_V, PH_R, PH_O, PH_D_, PH_E,
    5, PH_V, PH_O, PH_Z, PH_L_, PH_E,
    5, PH_V, PH_V_, PH_I, PH_D, PH_U,
    3, PH_V, PH_N_, PH_E,
    2, PH_V, PH_O,
    1, PH_V,
    1, PH_F,
    3, PH_D, PH_L_, PH_A,
    2, PH_D, PH_O,
    2, PH_I, PH_S,
    2, PH_I, PH_Z,
    2, PH_K, PH_O,
    1, PH_K,
    1, PH_G,
    5, PH_M_, PH_E, PH_ZH, PH_D, PH_U,
    3, PH_N, PH_A, PH_D,
    3, PH_N, PH_A, PH_T,
    2, PH_N, PH_A,
    2, PH_N_, PH_E,
    2, PH_O, PH_T,
    2, PH_O, PH_D,
    2, PH_O, PH_B,
    2, PH_O, PH_P,
    1, PH_O,
    5, PH_P_, PH_E, PH_R_, PH_E, PH_D,
    5, PH_P_, PH_E, PH_R_, PH_E, PH_T,
    5, PH_P, PH_O, PH_S, PH_L_, PH_E,
    3, PH_P, PH_O, PH_D,
    3, PH_P, PH_O, PH_T,
    3, PH_P, PH_R_, PH_I,
    3, PH_P, PH_R, PH_O,
    2, PH_P, PH_O,
    4, PH_R, PH_A, PH_D_, PH_I,
    5, PH_S, PH_K, PH_V, PH_O, PH_S_,
    5, PH_S, PH_K, PH_V, PH_O, PH_Z_,
    6, PH_S, PH_V_, PH_E, PH_R, PH_X, PH_A,
    2, PH_S, PH_O,
    1, PH_S,
    5, PH_Z, PH_Z, PH_A, PH_D_, PH_I,
    1, PH_U,
    5, PH_CH, PH_E, PH_R_, PH_E, PH_S,
    5, PH_CH, PH_E, PH_R_, PH_E, PH_Z,
    2, PH_Z, PH_A,
    1, PH_Z,
    0
  };

static const uint8_t seqlist3[] =
  {
    2, PH_B, PH_Y,
    1, PH_B,
    2, PH_ZH, PH_E,
    1, PH_ZH,
    5, PH_N_, PH_I, PH_B, PH_U, PH_D_,
    1, PH_P,
    2, PH_T, PH_O,
    0
  };

static const uint8_t seqlist4[] =
  {
    3, PH_A, PH_V, PH_A,
    3, PH_E, PH_V, PH_A,
    3, PH_A, PH_M, PH_U,
    3, PH_E, PH_M, PH_U,
    3, PH_I, PH_M_, PH_I,
    3, PH_A, PH_J, PH_E,
    3, PH_A, PH_J, PH_A,
    3, PH_I, PH_J, PH_E,
    3, PH_U, PH_J, PH_U,
    4, PH_O, PH_PRIMARY_STRESS, PH_V, PH_A,
    4, PH_E, PH_V, PH_O, PH_PRIMARY_STRESS,
    0
  };

static const uint8_t seqlist5[] =
  {
    2, PH_I, PH_J,
    3, PH_O, PH_PRIMARY_STRESS, PH_J,
    2, PH_E, PH_J,
    2, PH_I, PH_X,
    2, PH_I, PH_M,
    3, PH_O, PH_PRIMARY_STRESS, PH_M,
    2, PH_E, PH_M,
    0
  };


/* Local subroutines */

/*
 * Test a phoncode sequence pointed by ptr against specified list.
 * Returns non-zero when succeeded.
 */
static int test_list(const uint8_t *ptr, const uint8_t *lst)
{
  const uint8_t *item;
  for (item = lst; item[0] && memcmp(ptr, item + 1, item[0]); item += item[0] + 1);
  return (item[0] != 0) && (ptr[item[0]] > PH_TERMINATOR) && (ptr[item[0]] < PH_PRIMARY_STRESS);
}

/* Shift clause transcription one point left */
static void shift(uint8_t *buf)
{
  size_t i = 0;
  do
    {
      buf[i] = buf[i + 1];
      i++;
    }
  while ((buf[i] < PH_COMMA) || (buf[i] > PH_MINUS));
  buf[i] = PH_SPACE;
}

/*
 * Advance transcription up to the specified point.
 * Returns pointer to the transcription start.
 */
static uint8_t *transcription_advance(uint8_t *transcription, uint8_t *point)
{
  if (point > (transcription + TRANSCRIPTION_START))
    {
      size_t length = (point < (transcription + TRANSCRIPTION_BUFFER_SIZE)) ? (transcription + TRANSCRIPTION_BUFFER_SIZE - point) : 0;
      if (length)
        memmove(transcription + TRANSCRIPTION_START, point, length);
      memset(transcription + TRANSCRIPTION_START + length, PH_SPACE, TRANSCRIPTION_BUFFER_SIZE - TRANSCRIPTION_START - length);
    }
  return transcription + TRANSCRIPTION_START;
}

/* Synthesize speech for specified clause phonetic transcription */
static void synth_clause(uint8_t *transcription, ttscb_t *ttscb, uint8_t clause_type)
{
  soundscript_t *soundscript = malloc(sizeof(soundscript_t));
  if (soundscript)
    {
      time_plan_ptr_t draft;
      memset(soundscript, 0, sizeof(soundscript_t));
      soundscript->voice = (ttscb->flags & USE_ALTERNATIVE_VOICE) ? &female : &male;
      build_utterance(transcription, soundscript);
      draft = plan_time(transcription);
      if (draft)
        {
          apply_speechrate(soundscript, &(ttscb->timing), draft);
          free(draft);
        }
      apply_intonation(transcription, soundscript, &(ttscb->modulation), clause_type);
      make_sound(soundscript, &(ttscb->wave_consumer), ttscb->timing.rate_factor, !(ttscb->flags & USE_LEGACY_RATE_ALGO));
      free(soundscript);
    }
}

/* Synthesize speech clause by clause for specified phonetic transcription */
static void synth(uint8_t *transcription, ttscb_t *ttscb)
{
  uint8_t *tptr;
  uint8_t *sptr = transcription + TRANSCRIPTION_START;
  uint8_t count = 0;
  uint8_t flags = 4;

  for (tptr = transcription + TRANSCRIPTION_START; tptr < transcription + TRANSCRIPTION_BUFFER_SIZE; tptr++)
    {
      if (flags & 4)
        {
          flags &= ~4;
          if (test_list(tptr, seqlist1))
            {
              if (flags & 1)
                {
                  *sptr = PH_OPEN_BRACKET;
                  synth_clause(transcription, ttscb, 0);
                  tptr = transcription_advance(transcription, tptr);
                  count = 0;
                  flags &= ~1;
                  sptr = tptr;
                }
              flags |= 2;
              continue;
            }
          else if (test_list(tptr, seqlist2))
            {
              flags |= 2;
              continue;
            }
          else if (test_list(tptr, seqlist3) &&
                   (tptr > (transcription + TRANSCRIPTION_START)) &&
                   (*(tptr - 1) == PH_SPACE))
            {
              sptr = --tptr;
              shift(sptr);
              flags &= ~2;
              continue;
            }
        }
      if (*tptr != PH_SPACE)
        {
          if ((*tptr > PH_SPACE) && (*tptr < PH_PRIMARY_STRESS))
            {
              synth_clause(transcription, ttscb, ttscb->transcription_state.clause_type);
              break;
            }
        }
      else if (flags & 2)
        {
          shift(tptr--);
          flags = 4;
        }
      else if (((++count) != 3) || test_list(tptr + 1, seqlist1))
        {
          sptr = tptr;
          flags |= 5;
        }
      else
        {
          uint8_t *next;
          uint8_t perspective = 20;
          uint8_t k;
          if ((!test_list(tptr - 3, seqlist4)) && (!test_list(tptr - 2, seqlist5)))
            sptr = tptr;
          next = memchr(tptr + 1, PH_SPACE, perspective);
          if (next)
            perspective = (++next) - tptr;
          else next = tptr + perspective + 1;
          for (k = 1; k <= perspective; k++)
            if ((tptr[k] > PH_SPACE) && (tptr[k] < PH_PRIMARY_STRESS))
              break;
          if ((k > perspective) && !test_list(next, seqlist1))
            {
              *sptr = PH_OPEN_BRACKET;
              synth_clause(transcription, ttscb, 0);
              tptr = transcription_advance(transcription, sptr + 1) - 1;
              count = 0;
              flags &= ~2;
            }
          flags |= 5;
          sptr = tptr;
        }
    }
}


/* Global entry point */

/*
 * Transcription callback function.
 *
 * Finalizes provided transcription and performs speech synthesis for it.
 *
 * The buffer pointed by the first argument should contain a transcription
 * of length specified by the second argument. The last argument
 * should point to the corresponding ttscb structure.
 */
int synth_function(void *buffer, size_t length, void *user_data)
{
  ttscb_t *ttscb = user_data;
  if (length > TRANSCRIPTION_START)
    {
      if (ttscb->transcription_state.flags & CLAUSE_DONE)
        ttscb->transcription_state.flags &= ~CLAUSE_DONE;
      else
        {
          ((uint8_t *)buffer)[length] = 50;
          ttscb->transcription_state.clause_type = 0;
        }
      synth(buffer, ttscb);
    }
  return ttscb->wave_consumer.status;
}
