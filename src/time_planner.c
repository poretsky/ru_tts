/* time_planner.c -- Make up speech time layout
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
#include "soundscript.h"
#include "transcription.h"



/* Local macros */
#define MIN(x, y) ((x < y) ? x : y)
#define DISCRIMINANT(x, y) ((x != y) ? ((x != 1) ? 3 : 2) : 1)


/* Scratch data structure */
typedef struct
{
  uint8_t value;
  uint8_t delta;
  uint8_t flag;
  uint8_t ndx1;
  uint8_t ndx2;
  uint8_t itercount;
  uint8_t area[5][50];
} workspace_t;


/* Static data */

/* Phoncode sets for classification */
static const uint8_t set0[] = { PH_COMMA, PH_PERIOD, PH_QUESTION, PH_EXCLAMATION, PH_SEMICOLON, PH_COLON, PH_OPEN_BRACKET, PH_CLOSE_BRACKET };
static const uint8_t set1[] = { PH_J, PH_M, PH_M_, PH_N, PH_N_, PH_R, PH_R_, PH_L, PH_L_ };
static const uint8_t set2[] = { PH_V, PH_V_, PH_Z, PH_Z_, PH_ZH };
static const uint8_t set3[] = { PH_B, PH_B_, PH_D, PH_D_, PH_G, PH_G_ };
static const uint8_t set4[] = { PH_F, PH_F_, PH_S, PH_S_, PH_C, PH_SH, PH_SH_, PH_CH, PH_X, PH_X_ };
static const uint8_t set5[] = { PH_P, PH_P_, PH_T, PH_T_, PH_K, PH_K_ };
static const uint8_t set6[] = { PH_J, PH_L, PH_L_, PH_M, PH_M_, PH_N, PH_N_, PH_R, PH_R_ };
static const uint8_t set7[] = { PH_V, PH_V_, PH_Z, PH_Z_, PH_ZH, PH_B, PH_B_, PH_D, PH_D_, PH_G, PH_G_ };
static const uint8_t set8[] = { PH_B, PH_B_, PH_P, PH_P_, PH_M, PH_M_, PH_V, PH_V_, PH_F, PH_F_ };
static const uint8_t set9[] = { PH_D, PH_D_, PH_T, PH_T_, PH_L, PH_L_, PH_N, PH_N_, PH_Z, PH_Z_, PH_S, PH_S_, PH_SH, PH_SH_, PH_C, PH_CH, PH_ZH };
static const uint8_t set10[] = { PH_G, PH_G_, PH_K, PH_K_, PH_X, PH_X_ };


/* Local subroutines */

/* Define rank for specified phoncode */
static uint8_t rank(uint8_t phoncode)
{
  uint8_t rc = 0;
  if (memchr(set1, phoncode, sizeof(set1)))
    rc = 1;
  else if (memchr(set2, phoncode, sizeof(set2)))
    rc = 2;
  else if (memchr(set3, phoncode, sizeof(set3)))
    rc = 3;
  else if (memchr(set4, phoncode, sizeof(set4)))
    rc = 4;
  else if (memchr(set5, phoncode, sizeof(set5)))
    rc = 5;
  return rc;
}

/* Prepare next iteration */
static void next_iteration(workspace_t *scratch)
{
  scratch->area[3][++(scratch->ndx2)] = scratch->value;
  scratch->value = 0;
  if (scratch->flag)
    scratch->area[2][scratch->ndx1] += scratch->delta;
  scratch->delta = 0;
  scratch->itercount++;
}


/* Global entry point */

/*
 * Fill timing draft for specified phonetic transcription.
 *
 * This draft is used for speechrate applying.
 *
 * Returns non-zero value if the draft is successively filled.
 */
time_plan_ptr_t plan_time(uint8_t *transcription)
{
  time_plan_ptr_t draft;
  workspace_t *scratch;
  uint16_t i;
  uint8_t check_prev_trigger = 0;
  uint8_t check_prev = 0;
  uint8_t skip_itercount = 1;
  uint8_t nitems = 0;

  draft = calloc(TIME_PLAN_ROWS, sizeof(*draft));
  if (!draft)
    return NULL;

  scratch = malloc(sizeof(workspace_t));
  if (!scratch)
    {
      free(draft);
      return NULL;
    }
  memset(scratch, 0, sizeof(workspace_t));

  for (i = TRANSCRIPTION_START; i < TRANSCRIPTION_BUFFER_SIZE; i++)
    if (transcription[i] > PH_I)
      {
        if (transcription[i] != PH_SPACE)
          {
            uint8_t *found = memchr(set0, transcription[i], sizeof(set0));

            if (found)
              {
                uint8_t values[TIME_PLAN_ROWS];
                uint8_t restart = 0;
                uint8_t setcase = 0;
                uint8_t tmp = 0;
                uint8_t m = 1;
                uint8_t k = found - set0;
                uint8_t j;
                uint8_t ndx1 = 1;
                uint8_t ndx2 =1;
                uint8_t item;

                memset(values, 0, TIME_PLAN_ROWS);
                if (k > 3)
                  k = 0;
                values[8] = k + 1;
                next_iteration(scratch);
                skip_itercount = 1;
                scratch->area[4][++nitems] = scratch->itercount;
                scratch->itercount = 0;
                for (k = scratch->ndx2; k > 0; k--)
                  scratch->area[1][k] = 0;
                scratch->ndx1 = 0;
                scratch->ndx2 = 0;
                for (item = 1; item <= nitems; item++)
                  {
                    j = scratch->ndx2;
                    for (i = 1; i <= scratch->area[4][item]; i++)
                      {
                        uint8_t ndx3;
                        scratch->ndx2++;
                        for (ndx3 = 1; ndx3 <= scratch->area[3][scratch->ndx2]; ndx3++)
                          if (scratch->area[0][++(scratch->ndx1)])
                            {
                              int8_t l, n;
                              if (scratch->area[0][scratch->ndx1] == 1)
                                scratch->area[1][scratch->ndx2] = 1;
                              else if (scratch->area[0][scratch->ndx1] != 0xFF)
                                continue;
                              n = scratch->ndx1 - 2;
                              for (l = scratch->ndx1 - ndx3 + 1; l <= n; l++)
                                scratch->area[0][l] = 3;
                              if (ndx3 != 1)
                                scratch->area[0][scratch->ndx1 - 1] = 2;
                              n = scratch->area[3][scratch->ndx2] + scratch->ndx1 - ndx3;
                              for (l = scratch->ndx1 + 2; l <= n; l++)
                                scratch->area[0][l] = 5;
                              if (scratch->area[3][scratch->ndx2] != ndx3)
                                scratch->area[0][scratch->ndx1 + 1] = 4;
                              scratch->area[0][scratch->ndx1] = 1;
                            }
                      }
                    scratch->ndx2 = j;
                    k = scratch->area[4][item];
                    for (i = 1; i <= k; i++)
                      if (scratch->area[1][++(scratch->ndx2)] == 1)
                        {
                          int8_t l, n;
                          for (l = scratch->ndx2 - i + 1; l < scratch->ndx2; l++)
                            scratch->area[1][l] = 2;
                          n = scratch->area[4][item] + scratch->ndx2 - i;
                          for (l = scratch->ndx2 + 1; l <= n; l++)
                            scratch->area[1][l] = 3;
                          scratch->area[1][scratch->ndx2] = 1;
                        }
                  }

                scratch->ndx1 = 0;
                scratch->ndx2 = 0;
                values[7] = MIN(nitems, 4);
                i = TRANSCRIPTION_START - 1;
                for (item = 1; item <= nitems; item++)
                  {
                    uint8_t ndx4;
                    values[3] = DISCRIMINANT(item, nitems);
                    for (ndx4 = 1; ndx4 <= scratch->area[4][item]; ndx4++, ndx2++)
                      {
                        uint8_t ndx5;
                        scratch->ndx2++;
                        values[6] = MIN(scratch->area[3][ndx2], 4);
                        for (ndx5 = 1; ndx5 <= scratch->area[3][ndx2]; ndx5++, ndx1++)
                          {
                            scratch->ndx1++;
                            values[2] = DISCRIMINANT(ndx5, scratch->area[3][ndx2]);
                            for (j = 1; (j <= scratch->area[2][ndx1]) && (m < sizeof(*draft)); j++, m++)
                              {
                                uint8_t phoncode_cur = transcription[i];
                                uint8_t phoncode_prev = phoncode_cur;
                                while (1)
                                  {
                                    phoncode_cur = transcription[++i];
                                    if ((phoncode_cur < PH_V) || memchr(set6, phoncode_cur, sizeof(set6)))
                                      setcase = 2;
                                    else if (memchr(set7, phoncode_cur, sizeof(set7)))
                                      setcase = 3;
                                    else if ((phoncode_cur > PH_G_) && (phoncode_cur < PH_TERMINATOR))
                                      setcase = 4;
                                    else if ((phoncode_cur > PH_SPACE) && (phoncode_cur < PH_MINUS))
                                      {
                                        i = TRANSCRIPTION_START;
                                        restart = 1;
                                        values[0] = tmp;
                                        tmp = values[1];
                                        values[1] = 1;
                                        break;
                                      }
                                    else continue;
                                    values[0] = tmp;
                                    if (restart)
                                      {
                                        tmp = 0;
                                        restart = 0;
                                      }
                                    else tmp = values[1];
                                    values[1] = setcase;
                                    if (tmp)
                                      break;
                                    phoncode_prev = phoncode_cur;
                                  }
                                values[4] = scratch->area[0][scratch->ndx1];
                                values[5] = scratch->area[1][scratch->ndx2];
                                for (k = 2; k < TIME_PLAN_ROWS; k++)
                                  draft[k][m] = values[k] ? (values[k] - 1) : 0;
                                if ((phoncode_prev > PH_I) &&
                                    (phoncode_prev < PH_SPACE) &&
                                    (phoncode_prev == phoncode_cur))
                                  draft[1][m] = 5;
                                else if ((memchr(set8, phoncode_prev, 4) && memchr(set8, phoncode_cur, sizeof(set8))) ||
                                         (memchr(set9, phoncode_prev, 4) && memchr(set9, phoncode_cur, sizeof(set9))) ||
                                         (memchr(set10, phoncode_prev, 4) && memchr(set10, phoncode_cur, sizeof(set10))))
                                  draft[1][m] = 4;
                                else if (values[1])
                                  draft[1][m] = values[1] - 1;
                                else draft[1][m] = 0;
                                draft[0][m] = values[0] ? (values[0] - 1) : 3;
                              }
                          }
                      }
                  }
                free(scratch);
                return draft;
              }
            else
              {
                scratch->flag = 1;
                if (check_prev)
                  {
                    if (transcription[i - 1] != PH_SPACE)
                      {
                        uint8_t rank_prev;
                        uint8_t rank_cur;
                        check_prev = 0;
                        rank_prev = rank(transcription[i - 1]);
                        if (!rank_prev)
                          break;
                        rank_cur = rank(transcription[i]);
                        if (!rank_cur)
                          break;
                        if (rank_prev <= rank_cur)
                          scratch->area[2][scratch->ndx1]++;
                        else scratch->delta++;
                        continue;
                      }
                    else scratch->delta++;
                  }
                else scratch->delta++;
                if (check_prev_trigger)
                  {
                    check_prev = 1;
                    check_prev_trigger = 0;
                  }
              }
          }
        else next_iteration(scratch);
      }
    else
      {
        scratch->delta++;
        scratch->flag = 0;
        scratch->area[2][scratch->ndx1 + 1] = scratch->delta;
        if (transcription[i + 1] != PH_PRIMARY_STRESS)
          {
            if (transcription[i + 1] != PH_SECONDARY_STRESS)
              scratch->area[0][scratch->ndx1 + 1] = 0;
            else
              {
                scratch->area[0][scratch->ndx1 + 1] = 0xFF;
                i++;
              }
          }
        else
          {
            scratch->area[0][scratch->ndx1 + 1] = 1;
            if (skip_itercount)
              skip_itercount = 0;
            else
              {
                scratch->area[4][++nitems] = scratch->itercount;
                scratch->itercount = 0;
              }
            i++;
          }
        scratch->ndx1++;
        scratch->delta = 0;
        scratch->value++;
        check_prev_trigger = 1;
        check_prev = 0;
      }

  free(scratch);
  free(draft);
  return NULL;
}
