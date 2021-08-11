/* intonator.c -- Calculate and apply intonation
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

#include "transcription.h"
#include "soundscript.h"
#include "voice.h"


/* Static data */

/* Intonation plans for various clause types */
static const uint8_t intonations[][NSTAGES][2] =
  {
    {
      { 30, 40 },
      { 40, 50 },
      { 50, 50 },
      { 50, 40 },
      { 40, 50 },
      { 50, 60 },
      { 60, 50 },
      { 50, 40 },
      { 40, 40 },
      { 40, 60 },
      { 60, 70 },
      { 70, 70 }
    },
    {
      { 30, 40 },
      { 40, 50 },
      { 50, 50 },
      { 50, 40 },
      { 40, 45 },
      { 45, 50 },
      { 50, 60 },
      { 60, 40 },
      { 40, 25 },
      { 25, 15 },
      { 15, 30 },
      { 30, 60 }
    },
    {
      { 30, 60 },
      { 60, 70 },
      { 70, 60 },
      { 60, 40 },
      { 40, 50 },
      { 50, 60 },
      { 60, 50 },
      { 50, 40 },
      { 40, 40 },
      { 40, 60 },
      { 60, 70 },
      { 70, 70 }
    },
    {
      { 30, 60 },
      { 60, 70 },
      { 70, 60 },
      { 60, 40 },
      { 40, 50 },
      { 50, 60 },
      { 60, 50 },
      { 50, 40 },
      { 40, 40 },
      { 40, 60 },
      { 60, 70 },
      { 70, 70 }
    },
    {
      { 30, 60 },
      { 60, 70 },
      { 70, 60 },
      { 60, 40 },
      { 40, 50 },
      { 50, 60 },
      { 60, 50 },
      { 50, 40 },
      { 40, 40 },
      { 40, 60 },
      { 60, 70 },
      { 70, 70 }
    },
    {
      { 30, 40 },
      { 40, 50 },
      { 50, 60 },
      { 60, 70 },
      { 70, 20 },
      { 20, 40 },
      { 40, 60 },
      { 60, 40 },
      { 40, 50 },
      { 50, 20 },
      { 20, 0 },
      { 0, 0 }
    },
    {
      { 30, 30 },
      { 35, 40 },
      { 40, 40 },
      { 40, 40 },
      { 40, 20 },
      { 20, 30 },
      { 30, 40 },
      { 40, 40 },
      { 30, 20 },
      { 20, 0 },
      { 0, 0 },
      { 0, 0 }
    },
    {
      { 30, 40 },
      { 40, 50 },
      { 50, 60 },
      { 60, 60 },
      { 60, 20 },
      { 20, 40 },
      { 40, 60 },
      { 60, 60 },
      { 40, 60 },
      { 60, 35 },
      { 35, 20 },
      { 20, 0 }
    },
    {
      { 30, 45 },
      { 45, 65 },
      { 65, 80 },
      { 80, 80 },
      { 80, 40 },
      { 40, 60 },
      { 60, 80 },
      { 80, 80 },
      { 40, 80 },
      { 80, 50 },
      { 50, 20 },
      { 20, 0 }
    },
    {
      { 20, 30 },
      { 30, 25 },
      { 25, 25 },
      { 25, 20 },
      { 20, 30 },
      { 30, 25 },
      { 25, 25 },
      { 25, 20 },
      { 20, 30 },
      { 30, 25 },
      { 25, 20 },
      { 20, 10 }
    },
    {
      { 40, 80 },
      { 80, 80 },
      { 80, 80 },
      { 80, 70 },
      { 70, 70 },
      { 70, 65 },
      { 65, 60 },
      { 60, 60 },
      { 60, 80 },
      { 80, 100 },
      { 100, 100 },
      { 100, 10 }
    },
    {
      { 40, 80 },
      { 80, 90 },
      { 90, 90 },
      { 90, 80 },
      { 80, 80 },
      { 80, 80 },
      { 80, 80 },
      { 80, 80 },
      { 80, 60 },
      { 60, 40 },
      { 40, 20 },
      { 20, 0 }
    },
    {
      { 40, 80 },
      { 80, 80 },
      { 80, 80 },
      { 80, 70 },
      { 70, 70 },
      { 70, 65 },
      { 65, 60 },
      { 60, 60 },
      { 60, 80 },
      { 80, 100 },
      { 100, 100 },
      { 100, 10 }
    },
    {
      { 40, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 50 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 50, 80 },
      { 80, 100 },
      { 100, 40 },
      { 20, 0 }
    },
    {
      { 40, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 40, 80 },
      { 80, 100 },
      { 100, 20 },
      { 20, 0 }
    },
    {
      { 40, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 100 },
      { 100, 70 },
      { 70, 50 },
      { 50, 20 },
      { 20, 0 }
    }
  };


/* Local routines */

/*
 * Search transcription buffer for a speech breakpoint
 * ahead from specified index.
 *
 * Returns the last checked code or -1 if nothing found.
 */
static int search_breakpoint(uint8_t *transcription, int start_index)
{
  int rc = -1;
  int i;

  for (i = start_index; i < TRANSCRIPTION_BUFFER_SIZE; i++)
    {
      rc = transcription[i];
      if (rc != 43)
        {
          if (rc > 43)
            return -1;
          break;
        }
    }
  for (i++; i < TRANSCRIPTION_BUFFER_SIZE; i++)
    {
      rc = transcription[i];
      if (rc > 42)
        return rc;
    }
  return -1;
}

static unsigned int eval_tone(unsigned int x, uint16_t mintone, uint16_t maxtone)
{
  return 10000 / ((maxtone - mintone) * x / 100 + mintone);
}

/* Returns index where process was stopped */
static uint16_t setstage(soundscript_t *script, uint16_t start_index, uint8_t value)
{
  uint16_t i = start_index;

  while (i < script->length)
    {
      uint8_t prev = script->sounds[i].stage;
      script->sounds[i++].stage = value;
      if (script->sounds[i].stage <= prev)
        break;
    }
  return i;
}


/* Global entry point */

/* Apply intonation parameters to the sound script */
void apply_intonation(uint8_t *transcription, soundscript_t *soundscript,
                      uint16_t mintone, uint16_t maxtone, uint8_t clause_type)
{
  uint16_t i = TRANSCRIPTION_START;
  uint16_t nspeechmarks = 0;
  int bp;

  while (i < TRANSCRIPTION_BUFFER_SIZE)
    {
      bp = search_breakpoint(transcription, i);
      if (bp < 0)
        break;
      if (bp != 54)
        nspeechmarks++;
      while (((++i) < TRANSCRIPTION_BUFFER_SIZE) &&
             ((transcription[i] >= 53) || (transcription[i] < 43)));
    }

  for (i = 0; i < NSTAGES; i++)
    {
      soundscript->icb[i].count = 1;
      soundscript->icb[i].period = 1;
    }

  if (nspeechmarks)
    {
      uint16_t coef[NSTAGES];
      uint16_t prevk = 256;
      uint16_t j = 0;
      uint8_t m = 0;
      uint8_t st4 = 0;
      uint8_t stage;

      for (i = 0; i < NSTAGES; i++)
        {
          soundscript->icb[i].stretch = 90;
          soundscript->icb[i].delta = 0;
        }

      for (i = TRANSCRIPTION_START; j < soundscript->length; i++)
        {
          if (!m)
            {
              if (nspeechmarks == 1)
                stage = 8;
              else if (st4)
                stage = 4;
              else
                {
                  stage = 0;
                  st4 = 1;
                }
              bp = search_breakpoint(transcription, i);
              m = ((bp != 53) && (bp != 54)) ? 1 : 2;
            }

          if (m < 3)
            {
              if ((m < 2) && (transcription[i] > 5))
                {
                  j = setstage(soundscript, j, stage);
                  continue;
                }
              else if ((m > 1) && ((transcription[i] > 5) || (transcription[i + 1] != 53)))
                {
                  if (transcription[i] != 54)
                    j = setstage(soundscript, j, stage);
                  continue;
                }

              m = soundscript->sounds[j].stage;
              while (j < soundscript->length)
                {
                  uint8_t l = m;
                  if (m == 4)
                    m = 3;
                  soundscript->sounds[j++].stage = m + stage;
                  m = soundscript->sounds[j].stage;
                  if (l >= m)
                    break;
                }

              m = 3;
              continue;
            }

          if (m > 2)
            {
              uint8_t l = transcription[i];
              if (l < 53)
                {
                  if (l < 43)
                    j = setstage(soundscript, j, stage + 3);
                  else if (l != 43)
                    break;
                  else
                    {
                      j++;
                      bp = search_breakpoint(transcription, i + 1);
                      if (bp < 0)
                        break;
                      else if (bp != 54)
                        {
                          nspeechmarks--;
                          m = 0;
                        }
                    }
                }
            }
        }

      memset(coef, 0, sizeof(coef));
      for (i = 0; i < soundscript->length; i++)
        {
          uint8_t j = soundscript->sounds[i].id;
          uint8_t k = soundscript->sounds[i].stage;
          if (soundscript->voice->sound_lengths[j] < VOICE_THRESHOLD)
            {
              uint16_t l = soundscript->sounds[i].duration;
              l /= 10;
              coef[k] += l;
            }
        }

      for (i = 0; i < soundscript->length; i++)
        {
          uint8_t j = soundscript->sounds[i].id;
          uint8_t k = soundscript->sounds[i].stage;
          if ((prevk != ((uint16_t)k)) && (soundscript->voice->sound_lengths[j] < VOICE_THRESHOLD))
            {
              int32_t q = 0;
              uint8_t tone1 = eval_tone(intonations[clause_type][k][0], mintone, maxtone);
              uint8_t tone2 = eval_tone(intonations[clause_type][k][1], mintone, maxtone) - tone1;
              soundscript->icb[k].period = 1;
              soundscript->icb[k].count = 1;
              if (tone2)
                {
                  int32_t t;
                  int32_t r;
                  int8_t tone3 = tone2;
                  tone2 = tone1;
                  tone1 += tone3 >> 1;
                  t = ((((uint32_t)(coef[k])) * 10) + (tone1 >> 1)) / ((uint32_t)tone1);
                  if (!t)
                    t++;
                  q = ((int32_t)tone3) / t;
                  r = ((int32_t)tone3) % t;
                  if (q)
                    {
                      if (r < 0)
                        {
                          r = (-r) << 1;
                          if (r >= t)
                            q--;
                        }
                      else
                        {
                          r <<= 1;
                          if (r >= t)
                            q++;
                        }
                    }
                  else
                    {
                      q = (r < 0) ? -1 : 1;
                      if (r < 0)
                        r = -r;
                      t = (((uint32_t)t) + (r >> 1)) / r;
                      t &= 0xFF;
                      soundscript->icb[k].period = t;
                      soundscript->icb[k].count = t;
                    }
                }
              else tone2 = tone1;
              soundscript->icb[k].stretch = tone2;
              soundscript->icb[k].delta = q;
              prevk = (uint16_t)k;
            }
          else if (prevk != ((uint16_t)k))
            prevk = 256;
        }
    }
  else
    {
      for (i = 0; i < NSTAGES; i++)
        {
          soundscript->icb[i].stretch = VOICE_THRESHOLD;
          soundscript->icb[i].delta = 0;
        }

      for (i = 0; i < soundscript->length; i = setstage(soundscript, i, 0));
    }
}
