/* numerics.c -- Numbers phonetic transcription
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>

#include "numerics.h"
#include "transcription.h"
#include "sink.h"


/* Local constants */
#define NUMBER_FRACTION 1
#define NON_ZERO 2


/* Local data */

/* Predefined transcriptions */
static const uint8_t primary[] = /* 0..9 */
  {
    4, 16, 1, 53, 17,
    5, 2, 24, 5, 53, 16,
    4, 21, 6, 2, 53,
    4, 27, 13, 5, 53,
    7, 33, 3, 27, 4, 53, 13, 3,
    4, 29, 2, 53, 30,
    5, 36, 3, 53, 38, 30,
    4, 38, 3, 53, 15,
    6, 6, 1, 53, 38, 3, 15,
    6, 24, 3, 53, 11, 2, 30
  };
static const uint8_t secondary[] = /* 10..19 */
  {
    6, 24, 3, 53, 38, 2, 30,
    9, 2, 24, 5, 53, 16, 2, 32, 2, 30,
    9, 21, 11, 3, 16, 2, 53, 32, 2, 30,
    9, 27, 13, 5, 16, 2, 53, 32, 2, 30,
    11, 33, 3, 27, 4, 53, 8, 16, 2, 32, 2, 30,
    9, 29, 2, 27, 16, 2, 53, 32, 2, 30,
    9, 36, 3, 35, 16, 2, 53, 32, 2, 30,
    9, 38, 3, 15, 16, 2, 53, 32, 2, 30,
    11, 6, 2, 38, 3, 15, 16, 2, 53, 32, 2, 30,
    11, 24, 3, 11, 2, 27, 16, 2, 53, 32, 2, 30
  };
static const uint8_t tens[] = /* 20..90 */
  {
    7, 21, 6, 2, 53, 32, 2, 30,
    7, 27, 13, 5, 53, 32, 2, 30,
    6, 35, 1, 53, 8, 2, 28,
    8, 29, 2, 24, 3, 38, 2, 53, 27,
    9, 36, 3, 12, 24, 3, 38, 2, 53, 27,
    9, 38, 3, 53, 15, 24, 3, 38, 2, 27,
    11, 6, 1, 53, 38, 3, 15, 24, 3, 38, 2, 27,
    10, 24, 3, 11, 2, 16, 1, 53, 35, 27, 2
  };
static const uint8_t hundreds[] = /* 100..900 */
  {
    4, 35, 27, 1, 53,
    7, 21, 11, 3, 53, 38, 30, 5,
    7, 27, 13, 5, 53, 35, 27, 2,
    10, 33, 3, 27, 4, 53, 13, 3, 35, 27, 2,
    7, 29, 2, 27, 35, 1, 53, 27,
    7, 36, 3, 35, 35, 1, 53, 27,
    7, 38, 3, 15, 35, 1, 53, 27,
    9, 6, 2, 38, 3, 15, 35, 1, 53, 27,
    9, 24, 3, 11, 2, 27, 35, 1, 53, 27
  };
static const uint8_t periods[] =
  {
    6, 27, 4, 53, 38, 2, 33,
    7, 18, 5, 17, 5, 1, 53, 16,
    8, 18, 5, 17, 5, 2, 53, 8, 27,
    8, 27, 13, 5, 17, 5, 1, 53, 16
  };
static const uint8_t fractions[] =
  {
    6, 24, 3, 38, 2, 53, 27,
    4, 35, 1, 53, 27,
    7, 27, 4, 53, 38, 2, 33, 16,
    13, 24, 3, 38, 2, 30, 5, 27, 4, 53, 38, 2, 33, 16,
    10, 35, 27, 1, 27, 4, 53, 38, 2, 33, 16,
    8, 18, 5, 17, 5, 1, 53, 16, 16,
    14, 24, 3, 38, 2, 30, 5, 18, 5, 17, 5, 1, 53, 16, 16,
    11, 35, 27, 1, 18, 5, 17, 5, 1, 53, 16, 16,
    9, 18, 5, 17, 5, 2, 53, 8, 27, 16,
    15, 24, 3, 38, 2, 30, 5, 18, 5, 17, 5, 2, 53, 8, 27, 16,
    12, 35, 27, 1, 18, 5, 17, 5, 2, 53, 8, 27, 16,
    9, 27, 13, 5, 17, 5, 1, 53, 16, 16,
    15, 24, 3, 38, 2, 30, 5, 27, 13, 5, 17, 5, 1, 53, 16, 16,
    12, 35, 27, 1, 27, 13, 5, 17, 5, 1, 53, 16, 16
  };
static const uint8_t suffixes[] =
  {
    2, 4, 40,
    3, 2, 10, 2,
    2, 2, 6
  };
static const uint8_t one_int[] =
  {
    2, 21, 16, 2, 53, 43,
    32, 3, 53, 14, 2, 10, 2, 43
  };
static const uint8_t one_o[] = { 2, 21, 16, 1, 53, 43 };
static const uint8_t two_e[] = { 21, 11, 3, 53, 43 };
static const uint8_t n_ints[] = { 32, 3, 53, 14, 4, 40 };


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
    sink_put(consumer, 43);
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
      if (sink_last(consumer) != 43)
        sink_put(consumer, 43);
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
                  sink_put(consumer, 43);
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
                          else if ((s >= input->end) || (s[1] != '.') || !IS_DIGIT(s[2]))
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
                                   ((s[1] == '.') && IS_DIGIT(s[2])))
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
                  sink_put(consumer, 43);
                  break;
                }
            }
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
                              if (nc > 1)
                                sink_put(consumer, 2);
                              else if (nc != 1)
                                put_transcription(consumer, suffixes, 2);
                            }
                          else if (nc > 0)
                            sink_put(consumer, (nc > 1) ? 5 : 2);
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
          sink_put(consumer, 43);
          put_transcription(consumer, fractions, n - 1);
          put_transcription(consumer, suffixes, (nc != 1) ? 0 : 1);
          break;
        }
      else if (((input->start + 1) < input->end) && (input->start[0] == '.') && IS_DIGIT(input->start[1]))
        {
          flags |= NUMBER_FRACTION;
          sink_put(consumer, 43);
          if (nc != 1)
            {
              sink_write(consumer, n_ints, 6);
              sink_flush(consumer);
            }
        }
      else
        {
          sink_put(consumer, 43);
          break;
        }
    }
}
