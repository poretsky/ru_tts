/* numerics.c -- Numbers phonetic transcription
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

#include "phonemes.h"
#include "numerics.h"
#include "transcription.h"
#include "sink.h"
#include "synth.h"
#include "ru_tts.h"


/* Local constants */
#define NUMBER_FRACTION 1
#define NON_ZERO 2


/* Local data */

/* Predefined transcriptions */
static const uint8_t primary[] = /* 0..9 */
  {
    4, PH_N, PH_O, PH_PRIMARY_STRESS, PH_L_,
    5, PH_A, PH_D_, PH_I, PH_PRIMARY_STRESS, PH_N,
    4, PH_D, PH_V, PH_A, PH_PRIMARY_STRESS,
    4, PH_T, PH_R_, PH_I, PH_PRIMARY_STRESS,
    7, PH_CH, PH_E, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_R_, PH_E,
    4, PH_P_, PH_A, PH_PRIMARY_STRESS, PH_T_,
    5, PH_SH, PH_E, PH_PRIMARY_STRESS, PH_S_, PH_T_,
    4, PH_S_, PH_E, PH_PRIMARY_STRESS, PH_M,
    6, PH_V, PH_O, PH_PRIMARY_STRESS, PH_S_, PH_E, PH_M,
    6, PH_D_, PH_E, PH_PRIMARY_STRESS, PH_V_, PH_A, PH_T_
  };
static const uint8_t secondary[] = /* 10..19 */
  {
    6, PH_D_, PH_E, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_T_,
    9, PH_A, PH_D_, PH_I, PH_PRIMARY_STRESS, PH_N, PH_A, PH_C, PH_A, PH_T_,
    9, PH_D, PH_V_, PH_E, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    9, PH_T, PH_R_, PH_I, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    11, PH_CH, PH_E, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_R, PH_N, PH_A, PH_C, PH_A, PH_T_,
    9, PH_P_, PH_A, PH_T, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    9, PH_SH, PH_E, PH_S, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    9, PH_S_, PH_E, PH_M, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    11, PH_V, PH_A, PH_S_, PH_E, PH_M, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    11, PH_D_, PH_E, PH_V_, PH_A, PH_T, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_
  };
static const uint8_t tens[] = /* 20..90 */
  {
    7, PH_D, PH_V, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    7, PH_T, PH_R_ , PH_I, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    6, PH_S, PH_O, PH_PRIMARY_STRESS, PH_R, PH_A, PH_K,
    8, PH_P_, PH_A, PH_D_, PH_E, PH_S_, PH_A, PH_PRIMARY_STRESS, PH_T,
    9, PH_SH, PH_E, PH_Z_, PH_D_, PH_E, PH_S_, PH_A, PH_PRIMARY_STRESS, PH_T,
    9, PH_S_, PH_E, PH_PRIMARY_STRESS, PH_M, PH_D_, PH_E, PH_S_, PH_A, PH_T,
    11, PH_V, PH_O, PH_PRIMARY_STRESS, PH_S_, PH_E, PH_M, PH_D_, PH_E, PH_S_, PH_A, PH_T,
    10, PH_D_, PH_E, PH_V_, PH_A, PH_N, PH_O, PH_PRIMARY_STRESS, PH_S, PH_T, PH_A
  };
static const uint8_t hundreds[] = /* 100..900 */
  {
    4, PH_S, PH_T, PH_O, PH_PRIMARY_STRESS,
    7, PH_D, PH_V_, PH_E, PH_PRIMARY_STRESS, PH_S_, PH_T_, PH_I,
    7, PH_T, PH_R_, PH_I, PH_PRIMARY_STRESS, PH_S, PH_T, PH_A,
    10, PH_CH, PH_E, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_R_, PH_E, PH_S, PH_T, PH_A,
    7, PH_P_, PH_A, PH_T, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    7, PH_SH, PH_E, PH_S, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    7, PH_S_, PH_E, PH_M, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    9, PH_V, PH_A, PH_S_, PH_E, PH_M, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    9, PH_D_, PH_E, PH_V_, PH_A, PH_T, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T
  };
static const uint8_t periods[] =
  {
    6, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH,
    7, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N,
    8, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T,
    8, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N
  };
static const uint8_t fractions[] =
  {
    6, PH_D_, PH_E, PH_S_, PH_A, PH_PRIMARY_STRESS, PH_T,
    4, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    7, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH, PH_N,
    13, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH, PH_N,
    10, PH_S, PH_T, PH_O, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH, PH_N,
    8, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    14, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    11, PH_S, PH_T, PH_O, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    9, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T, PH_N,
    15, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T, PH_N,
    12, PH_S, PH_T, PH_O, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T, PH_N,
    9, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    15, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    12, PH_S, PH_T, PH_O, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N
  };
static const uint8_t suffixes[] =
  {
    2, PH_Y, PH_X,
    3, PH_A, PH_J, PH_A,
    2, PH_A, PH_F
  };
static const uint8_t one_int[] =
  {
    PH_A, PH_D, PH_N, PH_A, PH_PRIMARY_STRESS, PH_SPACE,
    PH_C, PH_E, PH_PRIMARY_STRESS, PH_L, PH_A, PH_J, PH_A, PH_SPACE
  };
static const uint8_t one_o[] = { PH_A, PH_D, PH_N, PH_O, PH_PRIMARY_STRESS, PH_SPACE };
static const uint8_t two_e[] = { PH_D, PH_V_, PH_E, PH_PRIMARY_STRESS, PH_SPACE };
static const uint8_t n_ints[] = { PH_C, PH_E, PH_PRIMARY_STRESS, PH_L, PH_Y, PH_X };


/* Local subroutines */

/* Pass specified list item to the consumer */
static void put_transcription(sink_t *consumer, const uint8_t *lst, uint8_t n)
{
  const uint8_t *item = list_item(lst, n);
  sink_write(consumer, item + 1, item[0]);
}

/* Output transcription for specified digit */
static void transcribe_digit(sink_t *consumer, char digit, char following)
{
  put_transcription(consumer, primary, digit - '0');
  if (following != ' ')
    sink_put(consumer, PH_SPACE);
}

/* Return true if specified character may be treated as decimal point. */
static int check_dec_point(sink_t *consumer, char c)
{
  int flags = ((ttscb_t *) (consumer->user_data))->flags;
  return ((flags & DEC_SEP_POINT) && (c == '.')) ||
    ((flags & DEC_SEP_COMMA) && (c == ','));
}


/* Global functions */

/* Transcribe numeric string from input and pass result to the consumer */
void process_number(input_t *input, sink_t *consumer)
{
  unsigned char c;
  uint8_t flags = 0;

  for (c = input->start[0]; IS_DIGIT(c) && (input->start < input->end); c = *(++input->start))
    {
      char *s;
      uint8_t digits = 1;
      uint8_t triplets = 0;
      uint8_t lzn = 0;
      uint8_t nc = 0;
      uint8_t n;

      flags &= ~NON_ZERO;
      if (sink_last(consumer) != PH_SPACE)
        sink_put(consumer, PH_SPACE);
      for (s = input->start + 1; s < input->end; s++)
        if (IS_DIGIT(s[0]))
          {
            if (++digits > 3)
              {
                digits = 1;
                if (++triplets > 4)
                  {
                    digits = 3;
                    triplets = 4;
                    break;
                  }
              }
            else if ((flags &NUMBER_FRACTION) && (triplets > 3) && (digits > 1))
              break;
          }
        else break;
      n = triplets * 3 + digits;

      s = input->start;
      for (input->start += n; !consumer->status; s++)
        {
          c = s[0];
          nc = 0;
          if (c != '0')
            flags |= NON_ZERO;
          else if (IS_DIGIT(s[1]))
            lzn++;
          if ((c != '0') || !((flags & NON_ZERO) || IS_DIGIT(s[1])))
            {
              lzn = 0;
              switch (digits)
                {
                case 3:
                  put_transcription(consumer, hundreds, c - '1');
                  sink_put(consumer, PH_SPACE);
                  break;
                case 1:
                  if (c == '1')
                    {
                      nc = 1;
                      switch (triplets)
                        {
                        case 1:
                          sink_write(consumer, one_int, 6);
                          break;
                        case 0:
                          if (s[2] == '+')
                            {
                              if (s[1] == 'A')
                                {
                                  input->start += 2;
                                  s = input->start;
                                  sink_write(consumer, one_int, (s[0] != ' ') ? 6 : 5);
                                  break;
                                }
                              else if (s[1] == 'O')
                                {
                                  input->start += 2;
                                  s = input->start;
                                  sink_write(consumer, one_o, (s[0] != ' ') ? 6 : 5);
                                  break;
                                }
                            }
                        default:
                          if (flags & NUMBER_FRACTION)
                            {
                              if ((s < input->end) && IS_DIGIT(s[2]))
                                transcribe_digit(consumer, c, s[0]);
                              else sink_write(consumer, one_int, 6);
                            }
                          else if ((s >= input->end) || !check_dec_point(consumer, s[1]) || !IS_DIGIT(s[2]))
                            transcribe_digit(consumer, c, s[0]);
                          else sink_write(consumer, one_int, 14);
                          break;
                        }
                      break;
                    }
                  else if (c < '5')
                    {
                      nc = 2;
                      if (c == '2')
                        {
                          if ((triplets == 0) && (s[2] == '+') && (s[1] == 'E'))
                            {
                              input->start += 2;
                              s = input->start;
                              sink_write(consumer, two_e, (s[0] != ' ') ? 5 : 4);
                              break;
                            }
                          else if ((triplets == 1) ||
                                   ((flags & NUMBER_FRACTION) && (s == (input->start - 1))) ||
                                   (check_dec_point(consumer, s[1]) && IS_DIGIT(s[2])))
                            {
                              sink_write(consumer, two_e, 5);
                              break;
                            }
                        }
                    }
                  transcribe_digit(consumer, c, s[0]);
                  break;
                default:
                  if (c == '1')
                    {
                      put_transcription(consumer, secondary, *(++s) - '0');
                      nc = 0;
                      digits--;
                    }
                  else put_transcription(consumer, tens, c - '2');
                  sink_put(consumer, PH_SPACE);
                  break;
                }
            }
          else if (!flags)
            transcribe_digit(consumer, c, s[0]);
          if (!(--digits))
            {
              if (lzn == 3)
                {
                  lzn = 0;
                  if (triplets)
                    {
                      digits = 3;
                      triplets--;
                    }
                  else
                    {
                      sink_back(consumer);
                      break;
                    }
                }
              else
                {
                  lzn = 0;
                  if (triplets)
                    {
                      if (flags & NON_ZERO)
                        {
                          put_transcription(consumer, periods, triplets - 1);
                          if (triplets != 1)
                            {
                              if (nc != 1)
                                {
                                  if (sink_last(consumer) == PH_T)
                                    sink_replace(consumer, PH_D);
                                  if (nc > 1)
                                    sink_put(consumer, PH_A);
                                  else put_transcription(consumer, suffixes, 2);
                                }
                            }
                          else if (nc > 0)
                            sink_put(consumer, (nc > 1) ? PH_I : PH_A);
                          sink_flush(consumer);
                        }
                      digits = 3;
                      triplets--;
                    }
                  else
                    {
                      sink_back(consumer);
                      break;
                    }
                }
            }
        }

      if (consumer->status)
        break;
      else if (flags & NUMBER_FRACTION)
        {
          sink_put(consumer, PH_SPACE);
          put_transcription(consumer, fractions, n - 1);
          put_transcription(consumer, suffixes, (nc != 1) ? 0 : 1);
          break;
        }
      else if (((input->start + 1) < input->end) && check_dec_point(consumer, input->start[0]) && IS_DIGIT(input->start[1]))
        {
          flags |= NUMBER_FRACTION;
          sink_put(consumer, PH_SPACE);
          if (nc != 1)
            {
              sink_write(consumer, n_ints, 6);
              sink_flush(consumer);
            }
        }
      else
        {
          sink_put(consumer, PH_SPACE);
          break;
        }
    }
}
