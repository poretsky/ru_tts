/* transcription.c -- Phonetic transcription related functions
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
#include "numerics.h"
#include "sink.h"


/* Internal flags */
#define CLAUSE_START 0x10
#define WEAK_STRESS 0x20


/* Local macros */
#define PAIR(a, b) ((((uint16_t)(a)) << 8) | (((uint16_t)(b)) & 0xFF))


/* Local data */

/* Main punctuations */
static const char *punctuations = ",.;:?!-";

/* Special symbols */
static const char *symbols = " ,.;:?!()-+=\"$%&*";

/* Accepted characters list */
static const char *char_list = "TNRLMDPZG^JH_WC[FOE\\UQYX]`a'-*()%\"/&$><@+=";

/* Special symbols treated as blanks */
static const char *blanks = "\t#'/<>@";

/* Internal letters representation table */
static const char *letters = "`ABCDEFGHIJKLMNOPQRSTU_VXYZWa[^]+=";

/* Phonetic letter classes */
static const char *consonants = "JMNRL^HC[WSPFTK_ZBVDG";
static const char *vocalics = "`EI\\QUaYOA";
static const char *ndts = "NDTS";
static const char *bgdjz = "BGD_Z";

/* Phoncodes */
static const uint8_t vocal_phoncodes[] = { 0, 3, 4, 1, 2 };
static const uint8_t ndts_soft_phs[] = { 19, 24, 30, 38 };
static const uint8_t hard_consonant_phs[] =
  {
    10, 15, 16, 8,  14, 33, 40, 32, 39,
    36, 35, 26, 34, 27, 28,
    9,  7,  20, 6,  21, 22
  };
static const uint8_t soft_consonant_phs[] =
  {
    10, 18, 19, 13, 17, 33, 41, 32, 39,
    36, 38, 29, 37, 30, 31,
    9,  12, 23, 11, 24, 25
  };

/* Predefined transcription blocks */
static const uint8_t transcription_blocks[] =
  {
    3, 27, 3, 53,
    3, 3, 53, 16,
    3, 3, 53, 8,
    3, 3, 53, 17,
    3, 3, 53, 15,
    3, 21, 3, 53,
    3, 26, 3, 53,
    3, 7, 3, 53,
    3, 22, 3, 53,
    3, 33, 3, 53,
    10, 5, 28, 8, 2, 53, 27, 28, 2, 10, 3,
    3, 40, 2, 53,
    3, 9, 3, 53,
    3, 36, 2, 53,
    3, 32, 3, 53,
    3, 39, 2, 53,
    3, 3, 53, 34,
    2, 1, 53,
    3, 10, 3, 53,
    3, 10, 1, 53,
    2, 0, 53,
    3, 10, 2, 53,
    2, 4, 53,
    13, 18, 2, 53, 40, 28, 5, 10, 43, 7, 16, 2, 53, 28,
    14, 27, 11, 1, 53, 8, 21, 4, 10, 43, 7, 16, 2, 53, 28,
    3, 10, 0, 53,
    2, 3, 53,
    8, 28, 2, 6, 4, 53, 33, 31, 5,
    0,
    8, 0, 15, 16, 1, 53, 9, 5, 30,
    15, 2, 27, 28, 8, 4, 53, 30, 43, 35, 28, 1, 53, 26, 28, 0,
    15, 7, 2, 28, 8, 4, 53, 30, 43, 35, 28, 1, 53, 26, 28, 0,
    10, 26, 8, 2, 32, 3, 53, 16, 27, 2, 34,
    8, 28, 2, 6, 4, 53, 33, 31, 5,
    5, 21, 8, 1, 53, 23,
    10, 2, 15, 26, 3, 8, 35, 3, 53, 16, 21,
    8, 21, 1, 53, 14, 2, 8, 2, 34,
    6, 20, 1, 53, 17, 36, 3,
    6, 18, 3, 53, 19, 36, 3,
    9, 26, 2, 8, 2, 53, 22, 8, 2, 34,
    5, 29, 17, 0, 53, 35,
    11, 8, 2, 6, 19, 2, 53, 10, 3, 27, 38, 2,

    4, 1, 53, 6, 2, /* 42: O+GO */
    4, 2, 6, 1, 53, /* 43: OGO+ */
    3, 2, 6, 2, /* 44: OGO */
    4, 3, 53, 6, 2, /* 45: E+GO (1) */
    4, 3, 6, 1, 53, /* 46: EGO+ (1) */
    3, 3, 6, 2, /* 47: EGO (1) */
    5, 10, 3, 53, 6, 2, /* 48: E+GO (2) */
    5, 10, 3, 6, 1, 53, /* 49: EGO+ (2) */
    4, 10, 3, 6, 2, /* 50: EGO (2) */
    3, 27, 35, 2 /* 51: TSA */
  };

/* Clause termination pairs */
static const uint16_t clause_terminations[] =
  {
    PAIR(',', ' '),
    PAIR(',', ','),
    PAIR(',', ';'),
    PAIR(',', ':'),
    PAIR(',', '-'),
    PAIR('.', ' '),
    PAIR('.', '.'),
    PAIR(';', ' '),
    PAIR(':', ' '),
    PAIR('.', ':'),
    PAIR('?', ' '),
    PAIR('?', '.'),
    PAIR('?', ','),
    PAIR('!', ' '),
    PAIR('!', '.'),
    PAIR('!', ',')
  };


/* Local subroutines */

/* Put specified block to the transcription buffer */
static void put_transcription_block(sink_t *consumer, uint8_t n)
{
  transcription_state_t *transcription = consumer->user_data;
  const uint8_t *block = list_item(transcription_blocks, n);
  uint8_t i;

  for (i = 1; i <= block[0]; i++)
    {
      uint8_t c = block[i];
      sink_put(consumer, ((c == 53) && (transcription->flags & WEAK_STRESS)) ? 54 : c);
    }
}

/*
 * Detect specified suffix in the input text
 * and make all necessary changes in the referenced control data.
 *
 * Returns non-zero value when the match succeeded.
 */
static int detect_suffix(input_t *input, transcription_state_t *transcription, const char *suffix)
{
  int rc = 0;
  uint8_t n = strlen(suffix);
  uint8_t i;

  transcription->flags &= ~WEAK_STRESS;
  for (i = 0; i < n; i++)
    if (input->start[i] != suffix[i])
      {
        if ((suffix[i] == '+') && (input->start[i] == '='))
          transcription->flags |= WEAK_STRESS;
        else break;
      }

  if ((i == n) && (((input->start + n) >= input->end) || !strchr(letters, input->start[n])))
    {
      input->start += n - 1;
      rc = 1;
    }

  return rc;
}

/*
 * Check if clause transcription is terminated. If it is the case,
 * terminates transcription and flushes it to the consumer.
 * Returns non-zero value on success.
 */
static int check_clause_termination(input_t *input, sink_t *consumer)
{
  transcription_state_t *transcription = consumer->user_data;
  char *s = memchr(symbols + 1, input->start[0], 6);
  int result = 0;

  if (s)
    {
      char c = *(input->start++);
      char nextc = (input->start < input->end) ? input->start[0] : ' ';
      uint16_t termination = PAIR(c, strchr(punctuations, nextc) ? nextc : ' ');
      uint8_t i;
      for (i = 0; (i < (sizeof(clause_terminations) / sizeof(uint16_t))) && (clause_terminations[i] != termination); i++);
      transcription->clause_type = i & 0x0F;
      transcription->flags |= CLAUSE_DONE;
      sink_put(consumer, s - symbols + 43);
      sink_flush(consumer);
      result = 1;
    }

  return result;
}

/*
 * Get consonant phoncode by index from specified set
 * voicifying the unvoiced ones.
 */
static uint8_t voicify(const uint8_t *phs, uint8_t idx)
{
  return phs[(idx < 15) ? (idx + 6) : idx];
}

/*
 * Get consonant phoncode by index from specified set
 * unvoicifying the voiced ones.
 */
static uint8_t unvoicify(const uint8_t *phs, uint8_t idx)
{
  return phs[(idx < 15) ? idx : (idx - 6)];
}

/*
 * Get hard consonant phoncode by its index
 * unvoicifying the voiced ones and changing if necessary
 * in accordance with the following character.
 */
static uint8_t unvoicify_hard(uint8_t idx, char following)
{
  return (((idx != 10) && (idx != 16)) || (following != 'W')) ?
    unvoicify(hard_consonant_phs, idx) : 36;
}

/*
 * Get correct consonant phoncode by its index
 * depending on the following character specified.
 */
static uint8_t correct_consonant(uint8_t idx, char following)
{
  if (memchr(consonants + 5, following, 10))
    return unvoicify_hard(idx, following);
  else if (strchr(bgdjz, following))
    return (((idx != 10) && (idx != 16)) || (following != '_')) ?
      voicify(hard_consonant_phs, idx) : 9;
  return ((idx != 16) || (following != '_')) ?
    hard_consonant_phs[idx] : 9;
}


/* Common functions */

/* Get an item from the list by its number */
const uint8_t *list_item(const uint8_t *lst, uint8_t n)
{
  const uint8_t *item = lst;
  uint8_t i;

  for (i = 0; i < n; i++)
    item += item[0] + 1;
  return item;
}

/* Transcribe specified text clause by clause and pass result to the consumer */
void process_text(const char *text, sink_t *consumer)
{
  input_t input;
  transcription_state_t *transcription = consumer->user_data;
  char *s;
  int accented = 0;

  input.text = strdup(text);
  if (!input.text)
    return;

  input.start = input.text;
  input.end = input.text;
  transcription->flags = 0;

  for (s = input.text; *s; s++)
    {
      unsigned char c = *s;

      switch (c)
        {
        case '\n':
        case '\r':
          c = '\r';
          break;
        case 'j':
        case 'J':
          c = '_';
          break;
        case 'q':
        case 'Q':
        case 'x':
        case 'X':
          c = 'K';
          break;
        case 'w':
        case 'W':
          c = 'U';
          break;
        case 'y':
        case 'Y':
          c = 'I';
          break;
        case 163:
        case 179:
          c = '\\';
          break;
        default:
          if (strchr(blanks, c))
            c = ' ';
          else if (c > 191)
            c = letters[(c - 192) & 31];
          else if ((c >= 'a') && (c <= 'z'))
            c -= 0x20;
          else if (((c < 'A') && !strchr(symbols, c) && !IS_DIGIT(c)) || (c > 'Z'))
            c = 0;
          break;
        }

      if (c)
        {
          const char *sptr = strchr(symbols, c);
          if (sptr)
            {
              int sidx = sptr - symbols;
              unsigned char nextc = s[1];
              if ((sidx > 6) ||
                  ((transcription->flags & CLAUSE_START) &&
                   ((c != ' ') || (nextc == '\r') ||
                    IS_DIGIT(nextc) || (nextc >= 'A'))))
                *(input.end)++ = c;
            }
          else
            {
              *(input.end)++ = c;
              transcription->flags |= CLAUSE_START;
            }
        }
    }
  *(input.end) = 0;

  for (s = input.start; (s < input.end) && (s[0] < 'A') && !IS_DIGIT(s[0]); s++);
  if (s >= input.end)
    {
      free(input.text);
      return;
    }

  while ((input.start < input.end) && !consumer->status)
    {
      unsigned char last_char = 0;

      while ((input.start < input.end) && memchr(symbols, input.start[0], 7))
        input.start++;
      sink_reset(consumer);
      for (transcription->flags = CLAUSE_START; (input.start < input.end) && (consumer->buffer_offset < TRANSCRIPTION_MAXLEN) && !consumer->status; input.start++)
        {
          char *s;
          unsigned char c = input.start[0];

          if (transcription->flags & CLAUSE_START)
            {
              accented = 0;
              for (s = input.start; s < input.end; s++)
                if ((s[0] == '+') || (s[0] == '='))
                  {
                    accented = 1;
                    break;
                  }
                else if (s[0] < 'A')
                  break;
            }

          s = strchr(char_list, c);
          if (s)
            {
              uint8_t char_index = s - char_list;
              if ((char_index < 17) &&
                  (last_char != '+') &&
                  (last_char != '=') &&
                  (last_char < 'A') &&
                  (input.start[1] < 'A'))
                {
                  put_transcription_block(consumer, char_index);
                  transcription->flags |= CLAUSE_START;
                  last_char = c;
                  continue;
                }
              else if (char_index > 26)
                {
                  uint8_t prev = sink_last(consumer);
                  if (((c != '+') && (c != '=')) || (prev > 5))
                    {
                      if ((prev < 43) || (prev > 52))
                        sink_put(consumer, 43);
                      put_transcription_block(consumer, char_index);
                      if ((c != '-') && (input.start[1] >= 'A'))
                        sink_put(consumer, 43);
                      transcription->flags |= CLAUSE_START;
                    }
                  else
                    {
                      sink_put(consumer, (c != '+') ? 54 : 53);
                      transcription->flags &= ~CLAUSE_START;
                    }
                  last_char = c;
                  continue;
                }
            }

          /* Some suffixes are transcribed specially when accented */
          if (accented)
            {
              if (detect_suffix(&input, transcription, "O+GO"))
                {
                  put_transcription_block(consumer, 42);
                  last_char = 'A';
                  continue;
                }
              else if (detect_suffix(&input, transcription, "E+GO"))
                {
                  s = input.start - 4;
                  put_transcription_block(consumer, ((s >= input.text) && strchr(consonants, *s)) ? 45 : 48);
                  last_char = 'A';
                  continue;
                }
              else if (detect_suffix(&input, transcription, "EGO+"))
                {
                  s = input.start - 4;
                  put_transcription_block(consumer, ((s >= input.text) && strchr(consonants, *s)) ? 46 : 49);
                  last_char = 'O';
                  continue;
                }
              else if (detect_suffix(&input, transcription, "OGO+"))
                {
                  put_transcription_block(consumer, 43);
                  last_char = 'O';
                  continue;
                }
              else if (detect_suffix(&input, transcription, "EGO"))
                {
                  s = input.start - 3;
                  put_transcription_block(consumer, ((s >= input.text) && strchr(consonants, *s)) ? 47 : 50);
                  last_char = 'A';
                  continue;
                }
              else if (detect_suffix(&input, transcription, "OGO"))
                {
                  put_transcription_block(consumer, 44);
                  last_char = 'A';
                  continue;
                }
              else if (detect_suffix(&input, transcription, "TSQ"))
                {
                  put_transcription_block(consumer, 51);
                  last_char = 'A';
                  continue;
                }
              else if (detect_suffix(&input, transcription, "TXSQ"))
                {
                  put_transcription_block(consumer, 51);
                  last_char = 'A';
                  continue;
                }
            }

          if (IS_DIGIT(c))
            {
              process_number(&input, consumer);
              if (!check_clause_termination(&input, consumer))
                sink_flush(consumer);
              transcription->flags |= CLAUSE_START;
              last_char = 0;
              input.start--;
              continue;
            }

          if (check_clause_termination(&input, consumer))
            break;

          s = strchr(vocalics, c);
          if (s)
            {
              uint8_t vc = (c == 'I') ? 5 :
                (((c == 'O') &&accented && (input.start[1] != '+') && (input.start[1] != '=')) ?
                 2 : vocal_phoncodes[(s - vocalics) % 5]);
              transcription->flags &= ~CLAUSE_START;
              if (input.start > input.text)
                {
                  unsigned char prevc = *(input.start - 1);
                  if (prevc != 'X')
                    {
                      if ((strchr(vocalics, prevc) || memchr(symbols, prevc, 13) ||
                           (prevc == ']')) &&
                          strchr("`QE\\", c))
                        sink_put(consumer, 10);
                    }
                  else if (strchr("`QE\\IO", c))
                    sink_put(consumer, 10);
                }
              else if (strchr("`QE\\", c))
                sink_put(consumer, 10);
              sink_put(consumer, vc);
              last_char = c;
              continue;
            }

          if (strchr(ndts, c))
            {
              unsigned char nextc = input.start[1];
              if (memchr(vocalics, nextc, 5) || (nextc == 'X'))
                {
                  s = strchr(ndts, last_char);
                  if (s)
                    sink_replace(consumer, ndts_soft_phs[s - ndts]);
                }
            }

          s = strchr(consonants, c);
          if (s)
            {
              unsigned char nextc = ((input.end - input.start) > 1) ? input.start[1] : ',';
              uint8_t idx = s - consonants;
              transcription->flags &= ~CLAUSE_START;
              if (idx < 9)
                {
                  if (nextc == 'X')
                    {
                      input.start++;
                      sink_put(consumer, soft_consonant_phs[idx]);
                    }
                  else sink_put(consumer, (memchr(vocalics, nextc, 5) ? soft_consonant_phs : hard_consonant_phs)[idx]);
                }
              else if (nextc == 'X')
                {
                  input.start++;
                  nextc = ((input.end - input.start) > 1) ? input.start[1] : ',';
                  if ((memchr(symbols + 1, nextc, 6) && (sink_last(consumer) != 43)) || memchr(consonants + 5, nextc, 10))
                    sink_put(consumer, unvoicify(soft_consonant_phs, idx));
                  else if (strchr(bgdjz, nextc))
                    sink_put(consumer, voicify(soft_consonant_phs, idx));
                  else sink_put(consumer, soft_consonant_phs[idx]);
                }
              else if (memchr(vocalics, nextc, 5))
                sink_put(consumer, soft_consonant_phs[idx]);
              else if (memchr(symbols + 1, nextc, 6))
                sink_put(consumer, (sink_last(consumer) != 43) ? unvoicify_hard(idx, nextc) : hard_consonant_phs[idx]);
              else sink_put(consumer, correct_consonant(idx, (nextc != ' ') ? nextc : input.start[2]));
            }
          else if (c != ']')
            {
              transcription->flags |= CLAUSE_START;
              sink_put(consumer, (c != '#') ? 43 : 42);
            }
          else transcription->flags &= ~CLAUSE_START;
          last_char = c;
        }
      sink_flush(consumer);
    }
  free(input.text);
}
