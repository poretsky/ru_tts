/* synth.c -- Phonetic transcription to speech transformation
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "synth.h"
#include "soundscript.h"
#include "transcription.h"
#include "timing.h"


/* Local data */

static const uint8_t seqlist1[] =
  {
    1, 2,
    5, 6, 8, 1, 24, 3,
    4, 21, 2, 25, 3,
    2, 21, 2,
    5, 10, 3, 21, 6, 2,
    5, 10, 3, 35, 17, 5,
    3, 5, 17, 5,
    4, 5, 27, 2, 28,
    5, 5, 16, 2, 33, 3,
    1, 5,
    3, 28, 2, 28,
    4, 17, 5, 20, 1,
    2, 16, 1,
    3, 8, 2, 35,
    5, 27, 2, 28, 25, 3,
    3, 27, 2, 28,
    3, 30, 3, 15,
    4, 27, 1, 25, 3,
    3, 40, 1, 30,
    3, 33, 3, 15,
    5, 33, 27, 1, 20, 4,
    4, 33, 27, 1, 26,
    3, 33, 27, 1,
    4, 5, 27, 2, 22,
    3, 28, 2, 22,
    3, 8, 2, 7,
    3, 27, 2, 22,
    3, 40, 1, 24,
    4, 33, 27, 1, 20,
    2, 27, 1,
    0
  };

static const uint8_t seqlist2[] =
  {
    3, 23, 3, 35,
    3, 23, 3, 7,
    4, 6, 21, 1, 17,
    4, 20, 17, 5, 35,
    4, 20, 17, 5, 7,
    5, 6, 8, 1, 24, 3,
    5, 6, 1, 7, 17, 3,
    5, 6, 11, 5, 21, 0,
    3, 6, 19, 3,
    2, 6, 1,
    1, 6,
    1, 34,
    3, 21, 17, 2,
    2, 21, 1,
    2, 5, 35,
    2, 5, 7,
    2, 28, 1,
    1, 28,
    1, 22,
    5, 18, 3, 9, 21, 0,
    3, 16, 2, 21,
    3, 16, 2, 27,
    2, 16, 2,
    2, 19, 3,
    2, 1, 27,
    2, 1, 21,
    2, 1, 20,
    2, 1, 26,
    1, 1,
    5, 29, 3, 13, 3, 21,
    5, 29, 3, 13, 3, 27,
    5, 26, 1, 35, 17, 3,
    3, 26, 1, 21,
    3, 26, 1, 27,
    3, 26, 13, 5,
    3, 26, 8, 1,
    2, 26, 1,
    4, 8, 2, 24, 5,
    5, 35, 28, 6, 1, 38,
    5, 35, 28, 6, 1, 12,
    6, 35, 11, 3, 8, 40, 2,
    2, 35, 1,
    1, 35,
    5, 7, 7, 2, 24, 5,
    1, 0,
    5, 33, 3, 13, 3, 35,
    5, 33, 3, 13, 3, 7,
    2, 7, 2,
    1, 7,
    0
  };

static const uint8_t seqlist3[] =
  {
    2, 20, 4,
    1, 20,
    2, 9, 3,
    1, 9,
    5, 19, 5, 20, 0, 24,
    1, 26,
    2, 27, 1,
    0
  };

static const uint8_t seqlist4[] =
  {
    3, 2, 6, 2,
    3, 3, 6, 2,
    3, 2, 15, 0,
    3, 3, 15, 0,
    3, 5, 18, 5,
    3, 2, 10, 3,
    3, 2, 10, 2,
    3, 5, 10, 3,
    3, 0, 10, 0,
    4, 1, 53, 6, 2,
    4, 3, 6, 1, 53,
    0
  };

static const uint8_t seqlist5[] =
  {
    2, 5, 10,
    3, 1, 53, 10,
    2, 3, 10,
    2, 5, 40,
    2, 5, 15,
    3, 1, 53, 15,
    2, 3, 15,
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
  return (item[0] != 0) && (ptr[item[0]] > 42) && (ptr[item[0]] < 53);
}

/* Shift transcription chunk */
static void shift(uint8_t *buf)
{
  size_t i = 0;
  do
    {
      buf[i] = buf[i + 1];
      i++;
    }
  while ((buf[i] < 44) || (buf[i] > 52));
  buf[i + 1] = 43;
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
      memset(transcription + TRANSCRIPTION_START + length, 43, TRANSCRIPTION_BUFFER_SIZE - TRANSCRIPTION_START - length);
    }
  return transcription + TRANSCRIPTION_START;
}

/* Synthesize speech for specified phonetic transcription */
static void synth_chunk(uint8_t *transcription, ttscb_t *ttscb, uint8_t clause_type)
{
  soundscript_t *soundscript = malloc(sizeof(soundscript_t));
  if (soundscript)
    {
      time_plan_ptr_t draft;
      memset(soundscript, 0, sizeof(soundscript_t));
      soundscript->voice = ttscb->voice;
      build_utterance(transcription, soundscript);
      draft = plan_time(transcription);
      if (draft)
        {
          apply_speechrate(soundscript, &(ttscb->timing), draft);
          free(draft);
        }
      apply_intonation(transcription, soundscript, ttscb->mintone, ttscb->maxtone, clause_type);
      make_sound(soundscript, &(ttscb->wave_consumer));
      free(soundscript);
    }
}


/* Global entry point */

/* Synthesize speech chunk by chunk for specified phonetic transcription */
void synth(uint8_t *transcription, ttscb_t *ttscb)
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
                  *sptr = 50;
                  synth_chunk(transcription, ttscb, 0);
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
                   (*(tptr - 1) == 43))
            {
              sptr = --tptr;
              shift(sptr);
              flags &= ~2;
              continue;
            }
        }
      if (*tptr != 43)
        {
          if ((*tptr > 43) && (*tptr < 53))
            {
              synth_chunk(transcription, ttscb, ttscb->transcription_state.clause_type);
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
          next = memchr(tptr + 1, 43, perspective);
          if (next)
            perspective = (++next) - tptr;
          else next = tptr + perspective + 1;
          for (k = 1; k <= perspective; k++)
            if ((tptr[k] > 43) && (tptr[k] < 53))
              break;
          if ((k > perspective) && !test_list(next, seqlist1))
            {
              *sptr = 50;
              synth_chunk(transcription, ttscb, 0);
              tptr = transcription_advance(transcription, sptr + 1) - 1;
              count = 0;
              flags &= ~2;
            }
          flags |= 5;
          sptr = tptr;
        }
    }
}
