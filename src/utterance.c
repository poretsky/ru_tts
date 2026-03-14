/* utterance.c -- Utterance builder
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

#include "soundscript.h"
#include "transcription.h"
#include "phonemes.h"


/* Local data */
static const uint8_t soundset1[] =
  {
    0, 0, 0, 0, 5, 5,
    5, 5, 0, 0, 0, 5,
    5, 5, 0, 0, 0, 5,
    5, 5, 0, 0, 0, 5,
    5, 5, 0, 5, 0, 0,
    0, 5, 5, 5, 0, 5
  };
static const uint8_t soundset2[] =
  {
    10, 20, 20, 20, 70, 50,
    60, 60, 40, 10, 20, 80,
    50, 60, 10, 20, 30, 50,
    60, 70, 10, 20, 30, 50,
    60, 70, 20, 60, 10, 20,
    20, 50, 60, 60, 30, 70
  };
static const uint8_t soundset3[] =
  {
    134, 134, 131,
    119, 119, 119
  };
static const uint8_t soundset4[] =
  {
    148, 148, 147,
    145, 145, 145
  };


/* Local subroutines */

/* Put specified sound into the soundscript */
static void put_sound(soundscript_t *script, uint8_t sound, uint8_t stage)
{
  script->sounds[script->length].id = sound;
  script->sounds[script->length++].stage = stage;
}


/* Global functions */

/* Build utterance according to phonetic transcription */
void build_utterance(uint8_t *transcription, soundscript_t *script)
{
  uint16_t i = TRANSCRIPTION_START;
  uint8_t a = PH_SPACE;
  uint8_t c = transcription[i];

  while ((a < PH_COMMA) && (i < TRANSCRIPTION_BUFFER_SIZE))
    {
      uint8_t flags = 0;
      uint16_t j;

      for (j = i; j < TRANSCRIPTION_BUFFER_SIZE; j++)
        if (transcription[j] != PH_SPACE)
          {
            if (transcription[j] < PH_SPACE)
              for (j++; j < TRANSCRIPTION_BUFFER_SIZE; j++)
                if (transcription[j] > PH_TERMINATOR)
                  {
                    if ((transcription[j] == PH_PRIMARY_STRESS) || (transcription[j] == PH_SECONDARY_STRESS))
                      flags |= 2;
                    break;
                  }
            break;
          }

      while (i < TRANSCRIPTION_BUFFER_SIZE)
        {
          uint8_t b = a;
          a = c;
          if (a > PH_SPACE)
            break;
          flags &= ~1;

          while (i < TRANSCRIPTION_BUFFER_SIZE)
            {
              c = transcription[++i];
              if (c < PH_PRIMARY_STRESS)
                break;
              flags |= 1;
            }

          if (a == PH_SPACE)
            {
              put_sound(script, 190, 2);
              break;
            }
          else if (a > PH_I)
            {
              if (a > PH_R_)
                {
                  if (a > PH_N_)
                    {
                      if (a > PH_K_)
                        {
                          if (a > PH_X_)
                            put_sound(script, 189, 2);
                          else if (a < PH_F)
                            {
                              put_sound(script, a + 143, 2);
                              put_sound(script, a + 145, 3);
                            }
                          else if ((a < PH_X) || (a == PH_X_) || (c > PH_V))
                            put_sound(script, a + 145, 2);
                          else put_sound(script, a + soundset4[c], 2);
                        }
                      else
                        {
                          put_sound(script, a + 143, 2);
                          if (a < PH_P_)
                            {
                              if (a < PH_P)
                                {
                                  if ((a < PH_B_) && (c < PH_V))
                                    put_sound(script, a + soundset3[c], 3);
                                  else put_sound(script, a + 119, 3);
                                }
                              else if (c < PH_V)
                                put_sound(script, a + soundset3[c], 3);
                              else put_sound(script, a + 119, 3);
                            }
                          else put_sound(script, a + 119, 3);
                        }
                    }
                  else put_sound(script, a + 119, 2);
                }
              else
                {
                  if (b > PH_R_)
                    put_sound(script, a + 99, 1);
                  if ((a != PH_J) || (transcription[i + 1] > PH_MINUS) || ((c > PH_I) && (c < PH_COMMA   )))
                    {
                      put_sound(script, a + 117, 2);
                      if (c > PH_R_)
                        put_sound(script, a + 99, 3);
                    }
                  else put_sound(script, 122, 2);
                }
            }
          else
            {
              j = 90;
              if (b > PH_I)
                {
                  if (b < PH_TERMINATOR)
                    j = soundset2[b - 6];
                  if (a == PH_I)
                    j--;
                }
              else j = (a != PH_I) ? 95 : 99;
              put_sound(script, a + j, 1);
              if (flags != 2)
                {
                  if ((b > PH_I) && (b < PH_TERMINATOR))
                    {
                      j = soundset1[b - 6];
                      if (a == PH_I)
                        j--;
                    }
                  else j = (a != PH_I) ? 0 : 4;
                  put_sound(script, a + j, 2);
                }
              if ((b > PH_I) && (b < PH_TERMINATOR))
                put_sound(script, a + soundset1[b - 6] + ((a != PH_I) ? 95 : 94), 3);
              else put_sound(script, a + ((a != PH_I) ? 95 : 99), 3);
              if (c > PH_I)
                {
                  if (c != PH_TERMINATOR)
                    {
                      if (c == PH_SPACE)
                        {
                          j = transcription[i + 1];
                          if (j > PH_I)
                            {
                              if (j < PH_TERMINATOR)
                                put_sound(script, a + soundset2[j - 6] + ((a != PH_I) ? 5 : 4), 4);
                              else put_sound(script, a + ((a != PH_I) ? 90 : 89), 4);
                            }
                          else if (b > PH_I)
                            put_sound(script, a + ((a != PH_I) ? 95 : 99), 4);
                        }
                      else if (c > PH_SPACE)
                        put_sound(script, a + ((a != PH_I) ? 90 : 89), 4);
                      else put_sound(script, a + soundset2[c - 6] + ((a != PH_I) ? 5 : 4), 4);
                    }
                  else put_sound(script, a + ((a != PH_I) ? 90 : 89), 4);
                }
              else if (b > PH_I)
                put_sound(script, a + ((a != PH_I) ? 95 : 99), 4);
            }
        }
    }

  if (i >= TRANSCRIPTION_BUFFER_SIZE)
    a = PH_COMMA;
  put_sound(script, a + 147, 2);
}
