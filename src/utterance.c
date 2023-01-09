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
  uint8_t a = 43;
  uint8_t c = transcription[i];

  while ((a < 44) && (i < TRANSCRIPTION_BUFFER_SIZE))
    {
      uint8_t flags = 0;
      uint16_t j;

      for (j = i; j < TRANSCRIPTION_BUFFER_SIZE; j++)
        if (transcription[j] != 43)
          {
            if (transcription[j] < 43)
              for (j++; j < TRANSCRIPTION_BUFFER_SIZE; j++)
                if (transcription[j] > 42)
                  {
                    if ((transcription[j] == 53) || (transcription[j] == 54))
                      flags |= 2;
                    break;
                  }
            break;
          }

      while (i < TRANSCRIPTION_BUFFER_SIZE)
        {
          uint8_t b = a;
          a = c;
          if (a > 43)
            break;
          flags &= ~1;

          while (i < TRANSCRIPTION_BUFFER_SIZE)
            {
              c = transcription[++i];
              if (c < 53)
                break;
              flags |= 1;
            }

          if (a == 43)
            {
              put_sound(script, 190, 2);
              break;
            }
          else if (a > 5)
            {
              if (a > 13)
                {
                  if (a > 19)
                    {
                      if (a > 31)
                        {
                          if (a > 41)
                            put_sound(script, 189, 2);
                          else if (a < 34)
                            {
                              put_sound(script, a + 143, 2);
                              put_sound(script, a + 145, 3);
                            }
                          else if ((a < 40) || (a == 41) || (c > 5))
                            put_sound(script, a + 145, 2);
                          else put_sound(script, a + soundset4[c], 2);
                        }
                      else
                        {
                          put_sound(script, a + 143, 2);
                          if (a < 29)
                            {
                              if (a < 26)
                                {
                                  if ((a < 23) && (c < 6))
                                    put_sound(script, a + soundset3[c], 3);
                                  else put_sound(script, a + 119, 3);
                                }
                              else if (c < 6)
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
                  if (b > 13)
                    put_sound(script, a + 99, 1);
                  if ((a != 10) || (transcription[i + 1] > 52) || ((c > 5) && (c < 44)))
                    {
                      put_sound(script, a + 117, 2);
                      if (c > 13)
                        put_sound(script, a + 99, 3);
                    }
                  else put_sound(script, 122, 2);
                }
            }
          else
            {
              j = 90;
              if (b > 5)
                {
                  if (b < 42)
                    j = soundset2[b - 6];
                  if (a == 5)
                    j--;
                }
              else j = (a != 5) ? 95 : 99;
              put_sound(script, a + j, 1);
              if (flags != 2)
                {
                  if ((b > 5) && (b < 42))
                    {
                      j = soundset1[b - 6];
                      if (a == 5)
                        j--;
                    }
                  else j = (a != 5) ? 0 : 4;
                  put_sound(script, a + j, 2);
                }
              if ((b > 5) && (b < 42))
                put_sound(script, a + soundset1[b - 6] + ((a != 5) ? 95 : 94), 3);
              else put_sound(script, a + ((a != 5) ? 95 : 99), 3);
              if (c > 5)
                {
                  if (c != 42)
                    {
                      if (c == 43)
                        {
                          j = transcription[i + 1];
                          if (j > 5)
                            {
                              if (j < 42)
                                put_sound(script, a + soundset2[j - 6] + ((a != 5) ? 5 : 4), 4);
                              else put_sound(script, a + ((a != 5) ? 90 : 89), 4);
                            }
                          else if (b > 5)
                            put_sound(script, a + ((a != 5) ? 95 : 99), 4);
                        }
                      else if (c > 43)
                        put_sound(script, a + ((a != 5) ? 90 : 89), 4);
                      else put_sound(script, a + soundset2[c - 6] + ((a != 5) ? 5 : 4), 4);
                    }
                  else put_sound(script, a + ((a != 5) ? 90 : 89), 4);
                }
              else if (b > 5)
                put_sound(script, a + ((a != 5) ? 95 : 99), 4);
            }
        }
    }

  if (i >= TRANSCRIPTION_BUFFER_SIZE)
    a = 44;
  put_sound(script, a + 147, 2);
}
