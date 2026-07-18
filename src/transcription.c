/* transcription.c -- Phonetic transcription related functions
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

#include "transcription.h"
#include "numerics.h"
#include "sink.h"
#include "phonemes.h"


/* Internal flags */
#define CLAUSE_START 0x10
#define WEAK_STRESS 0x20


/* Local macros */
#define PAIR(a, b) ((((uint16_t)(a)) << 8) | (((uint16_t)(b)) & 0xFF))
#ifdef _MSC_VER
#define strdup(p) _strdup(p)
#endif


/* Main punctuations */
const char *punctuations = ",.;:?!-";


/* Local data */

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
static const uint8_t vocal_phoncodes[] = { PH_U, PH_E, PH_Y, PH_O, PH_A };
static const uint8_t ndts_soft_phs[] = { PH_N_, PH_D_, PH_T_, PH_S_ };
static const uint8_t hard_consonant_phs[] =
  {
    PH_J, PH_M, PH_N, PH_R,  PH_L, PH_CH, PH_X, PH_C, PH_SH_,
    PH_SH, PH_S , PH_P, PH_F, PH_T, PH_K ,
    PH_ZH,  PH_Z,  PH_B, PH_V,  PH_D, PH_G
  };
static const uint8_t soft_consonant_phs[] =
  {
    PH_J, PH_M_, PH_N_, PH_R_, PH_L_, PH_CH, PH_X_, PH_C, PH_SH_,
    PH_SH, PH_S_, PH_P_, PH_F_, PH_T_, PH_K_,
    PH_ZH,  PH_Z_, PH_B_, PH_V_, PH_D_, PH_G_
  };

/* Predefined transcription blocks */
static const uint8_t transcription_blocks[] =
  {
    3, PH_T, PH_E, PH_PRIMARY_STRESS,
    3, PH_E, PH_PRIMARY_STRESS, PH_N,
    3, PH_E, PH_PRIMARY_STRESS, PH_R,
    3, PH_E, PH_PRIMARY_STRESS, PH_L_,
    3, PH_E, PH_PRIMARY_STRESS, PH_M,
    3, PH_D, PH_E, PH_PRIMARY_STRESS,
    3, PH_P, PH_E, PH_PRIMARY_STRESS,
    3, PH_Z, PH_E, PH_PRIMARY_STRESS,
    3, PH_G, PH_E, PH_PRIMARY_STRESS,
    3, PH_CH, PH_E, PH_PRIMARY_STRESS,
    10, PH_I, PH_K, PH_R, PH_A, PH_PRIMARY_STRESS, PH_T, PH_K, PH_A, PH_J, PH_E,
    3, PH_X, PH_A, PH_PRIMARY_STRESS,
    3, PH_ZH, PH_E, PH_PRIMARY_STRESS,
    3, PH_SH, PH_A, PH_PRIMARY_STRESS,
    3, PH_C, PH_E, PH_PRIMARY_STRESS,
    3, PH_SH_, PH_A, PH_PRIMARY_STRESS,
    3, PH_E, PH_PRIMARY_STRESS, PH_F,
    2, PH_O , PH_PRIMARY_STRESS,
    3, PH_J, PH_E , PH_PRIMARY_STRESS,
    3, PH_J, PH_O , PH_PRIMARY_STRESS,
    2, PH_U, PH_PRIMARY_STRESS,
    3, PH_J, PH_A, PH_PRIMARY_STRESS,
    2, PH_Y, PH_PRIMARY_STRESS,
    13, PH_M_, PH_A, PH_PRIMARY_STRESS, PH_X_, PH_K , PH_I, PH_J,
        PH_SPACE, PH_Z, PH_N, PH_A, PH_PRIMARY_STRESS, PH_K,
    14, PH_T, PH_V_, PH_O, PH_PRIMARY_STRESS, PH_R, PH_D, PH_Y, PH_J,
        PH_SPACE, PH_Z, PH_N, PH_A, PH_PRIMARY_STRESS, PH_K,
    3, PH_J, PH_U, PH_PRIMARY_STRESS,
    2, PH_E, PH_PRIMARY_STRESS,
    8, PH_K, PH_A, PH_V, PH_Y, PH_PRIMARY_STRESS, PH_CH, PH_K_, PH_I,
    0,
    8, PH_U, PH_M, PH_N, PH_O, PH_PRIMARY_STRESS, PH_ZH, PH_I, PH_T_,
    15, PH_A, PH_T, PH_K, PH_R, PH_Y, PH_PRIMARY_STRESS, PH_T_,
        PH_SPACE, PH_S, PH_K, PH_O, PH_PRIMARY_STRESS, PH_P, PH_K, PH_U,
    15, PH_Z, PH_A, PH_K, PH_R, PH_Y, PH_PRIMARY_STRESS, PH_T_,
        PH_SPACE, PH_S, PH_K, PH_O, PH_PRIMARY_STRESS, PH_P, PH_K, PH_U,
    10, PH_P, PH_R, PH_A, PH_C, PH_E, PH_PRIMARY_STRESS, PH_N, PH_T, PH_A, PH_F,
    8, PH_K, PH_A, PH_V, PH_Y, PH_PRIMARY_STRESS, PH_CH, PH_K_, PH_I,
    5, PH_D, PH_R, PH_O, PH_PRIMARY_STRESS, PH_B_,
    10, PH_A, PH_M, PH_P, PH_E, PH_R, PH_S, PH_E, PH_PRIMARY_STRESS, PH_N, PH_D,
    8, PH_D, PH_O, PH_PRIMARY_STRESS, PH_L, PH_A, PH_R, PH_A, PH_F,
    6, PH_B, PH_O , PH_PRIMARY_STRESS, PH_L_, PH_SH , PH_E,
    6, PH_M_, PH_E, PH_PRIMARY_STRESS, PH_N_, PH_SH , PH_E,
    9, PH_P, PH_A, PH_R, PH_A, PH_PRIMARY_STRESS, PH_G, PH_R, PH_A, PH_F,
    5, PH_P_, PH_L_, PH_U, PH_PRIMARY_STRESS, PH_S,
    11, PH_R, PH_A, PH_V, PH_N_, PH_A, PH_PRIMARY_STRESS, PH_J, PH_E, PH_T, PH_S_, PH_A,

    4, PH_O, PH_PRIMARY_STRESS, PH_V, PH_A , /* 42: O+GO */
    4, PH_A , PH_V, PH_O, PH_PRIMARY_STRESS, /* 43: OGO+ */
    3, PH_A, PH_V, PH_A, /* 44: OGO */
    4, PH_E, PH_PRIMARY_STRESS, PH_V, PH_A, /* 45: E+GO (1) */
    4, PH_E, PH_V, PH_O, PH_PRIMARY_STRESS, /* 46: EGO+ (1) */
    3, PH_E, PH_V, PH_A, /* 47: EGO (1) */
    5, PH_J, PH_E, PH_PRIMARY_STRESS, PH_V, PH_A, /* 48: E+GO (2) */
    5, PH_J, PH_E, PH_V, PH_O, PH_PRIMARY_STRESS, /* 49: EGO+ (2) */
    4, PH_J, PH_E, PH_V, PH_A, /* 50: EGO (2) */
    3, PH_T, PH_S, PH_A /* 51: TSA */
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
      sink_put(consumer, ((c == PH_PRIMARY_STRESS) && (transcription->flags & WEAK_STRESS)) ? PH_SECONDARY_STRESS : c);
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
      sink_put(consumer, s - symbols + PH_SPACE);
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
  return phs[(idx < PH_M) ? (idx + 6) : idx];
}

/*
 * Get consonant phoncode by index from specified set
 * unvoicifying the voiced ones.
 */
static uint8_t unvoicify(const uint8_t *phs, uint8_t idx)
{
  return phs[(idx < PH_M) ? idx : (idx - 6)];
}

/*
 * Get hard consonant phoncode by its index
 * unvoicifying the voiced ones and changing if necessary
 * in accordance with the following character.
 */
static uint8_t unvoicify_hard(uint8_t idx, char following)
{
  return (((idx != PH_J) && (idx != PH_N)) || (following != 'W')) ?
    unvoicify(hard_consonant_phs, idx) : PH_SH;
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
    return (((idx != PH_J) && (idx != PH_N)) || (following != '_')) ?
      voicify(hard_consonant_phs, idx) : PH_ZH;
  return ((idx != PH_N) || (following != '_')) ?
    hard_consonant_phs[idx] : PH_ZH;
}

/* Transcription cycle initialization actions */
static void transcription_init(sink_t *consumer)
{
  uint8_t *buffer = consumer->buffer;
  memset(buffer, PH_SPACE, TRANSCRIPTION_BUFFER_SIZE);
  consumer->buffer_offset = TRANSCRIPTION_START;
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

  consumer->custom_reset = transcription_init;
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
                  if (((c != '+') && (c != '=')) || (prev > PH_I))
                    {
                      if ((prev < PH_SPACE) || (prev > PH_MINUS))
                        sink_put(consumer, PH_SPACE);
                      put_transcription_block(consumer, char_index);
                      if ((c != '-') && (input.start[1] >= 'A'))
                        sink_put(consumer, PH_SPACE);
                      transcription->flags |= CLAUSE_START;
                    }
                  else
                    {
                      sink_put(consumer, (c != '+') ? PH_SECONDARY_STRESS  : PH_PRIMARY_STRESS);
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
              sink_flush(consumer);
              process_number(&input, consumer);
              if (check_clause_termination(&input, consumer))
                transcription->flags |= CLAUSE_START;
              else sink_flush(consumer);
              last_char = ' ';
              if (input.start[0] != ' ')
                input.start--;
              continue;
            }

          if (check_clause_termination(&input, consumer))
            break;

          s = strchr(vocalics, c);
          if (s)
            {
              uint8_t vc = (c == 'I') ? PH_I :
                (((c == 'O') &&accented && (input.start[1] != '+') && (input.start[1] != '=')) ?
                 PH_A : vocal_phoncodes[(s - vocalics) % 5]);
              transcription->flags &= ~CLAUSE_START;
              if (input.start > input.text)
                {
                  unsigned char prevc = *(input.start - 1);
                  if (prevc != 'X')
                    {
                      if ((strchr(vocalics, prevc) || memchr(symbols, prevc, 13) ||
                           (prevc == ']')) &&
                          strchr("`QE\\", c))
                        sink_put(consumer, PH_J);
                    }
                  else if (strchr("`QE\\IO", c))
                    sink_put(consumer, PH_J);
                }
              else if (strchr("`QE\\", c))
                sink_put(consumer, PH_J);
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
                  if ((memchr(symbols + 1, nextc, 6) && (sink_last(consumer) != PH_SPACE)) || memchr(consonants + 5, nextc, 10))
                    sink_put(consumer, unvoicify(soft_consonant_phs, idx));
                  else if (strchr(bgdjz, nextc))
                    sink_put(consumer, voicify(soft_consonant_phs, idx));
                  else sink_put(consumer, soft_consonant_phs[idx]);
                }
              else if (memchr(vocalics, nextc, 5))
                sink_put(consumer, soft_consonant_phs[idx]);
              else if (memchr(symbols + 1, nextc, 6))
                sink_put(consumer, (sink_last(consumer) != PH_SPACE) ? unvoicify_hard(idx, nextc) : hard_consonant_phs[idx]);
              else sink_put(consumer, correct_consonant(idx, (nextc != ' ') ? nextc : input.start[2]));
            }
          else if (c != ']')
            {
              transcription->flags |= CLAUSE_START;
              sink_put(consumer, (c != '#') ? PH_SPACE : PH_TERMINATOR);
            }
          else transcription->flags &= ~CLAUSE_START;
          last_char = c;
        }
      sink_flush(consumer);
    }
  free(input.text);
}
