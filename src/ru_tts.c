/* The Russian text to speech converter
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <locale.h>
#include <ctype.h>
#include <rulexdb.h>

#include "ru_tts.h"

#define WAVE_SIZE 4096

static const char *charset = "ru_RU.koi8r";
static const char *symbols = "+=ÁÂ×ÇÄÅ£ÖÚÉÊËÌÍÎÏÐÒÓÔÕÆÈÃÞÛÝßÙØÜÀÑ";
static const char *alphabet;

static int wave_consumer(void *buffer, size_t size, void *user_data)
{
  (void) user_data;
  (void)write(1, buffer, size);
  return 0;
}

static void *xrealloc(void *p, unsigned int n)
{
  void *u = realloc(p, n);
  if (u) return u;
  else
    {
      perror("Memory allocation error");
      exit(EXIT_FAILURE);
    }
}

static void *xmalloc(unsigned int n)
{
  void *u = NULL;
  return xrealloc(u, n);
}

static double getval(char *s)
{
  double val = atof(s);
  if ((val >= 0.0) && (val <= 1.0)) return val;
  fprintf(stderr, "Illegal option value\n");
  exit(EXIT_FAILURE);
}

static void usage(const char* name)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "%s [-s stress_db [-l logfile]] [-r rate] [-p pitch] [-g gaplen] [-m] [-a]\n", name);
  fprintf(stderr, "All numeric values must be from 0.0 to 1.0\n");
}

int main(int argc, char **argv)
{
  ru_tts_conf_t tts_params;
  size_t size = 64;
  char c, *s, *text, *input;
  void *wave;
  FILE *slog = NULL;
  RULEXDB *db = NULL;

  tts_params.speech_rate = 130;
  tts_params.voice_pitch = 50;
  tts_params.gap_factor = 80;
  tts_params.intonation = 1;
  tts_params.alternative_voice = 0;

  while ((c = getopt(argc, argv, "s:l:r:p:g:mah")) != -1)
    {
      switch (c)
        {
          case 's':
            db = rulexdb_open(optarg, RULEXDB_SEARCH);
            if (!db) perror(optarg);
            break;
          case 'l':
            slog = fopen(optarg, "a");
            if (!slog) perror(optarg);
            break;
          case 'm':
            tts_params.intonation = 0;
            break;
          case 'a':
            tts_params.alternative_voice = 1;
            break;
          case 'r':
            tts_params.speech_rate = 250 - rint(getval(optarg) * 250.0);
            break;
          case 'p':
            tts_params.voice_pitch = rint(getval(optarg) * 250.0);
            break;
          case 'g':
            tts_params.gap_factor = rint(getval(optarg) * 100.0);
            break;
          case 'h':
            usage(argv[0]);
            return EXIT_SUCCESS;
          default:
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

  /* Set locale if necessary */
  if (db)
    {
      alphabet = symbols + 2;
      if (!setlocale(LC_CTYPE, charset))
        {
          fprintf(stderr, "Cannot set \"%s\" locale.\n", charset);
          return EXIT_FAILURE;
        }
    }

  /* doing tts in the loop */
  wave = xmalloc(WAVE_SIZE);
  text = xmalloc(size);
  s = text;
  do
    {
      input = fgets(s, text + size - s, stdin);
      if (!input)
        strcpy(s, "\n");
      s = text + strlen(text) - 1;
      if (*s == '\n')
        {
          for (*s-- = 0; *text; s--)
            {
              if (*s == ' ') *s = 0;
              else break;
            }
          if (db)
            {
              char *stressed = xmalloc(strlen(text) << 1);
              char *t;
              unsigned int n;
              for (s = text; *s; s++)
                if (isupper(*s))
                  *s = tolower(*s);
              s = text;
              t = stressed;
              while (*s)
                {
                  if ((n = strcspn(s, symbols)))
                    {
                      strncpy(t, s, n);
                      s += n;
                      t += n;
                    }
                  if ((n = strspn(s, symbols)))
                    {
                      if ((n <= RULEXDB_MAX_KEY_SIZE) && (n <= strspn(s, alphabet)))
                        {
                          char *key = xmalloc(RULEXDB_BUFSIZE);
                          strncpy(key, s, n);
                          key[n] = 0;
                          if (rulexdb_search(db, key, t, 0)
                              == RULEXDB_SPECIAL)
                            if (slog) (void)fprintf(slog, "%s\n", key);
                          free(key);
                        }
                      else
                        {
                          strncpy(t, s, n);
                          t[n] = 0;
                        }
                      s += n;
                      t += strlen(t);
                    }
                }
              *t = 0;
              ru_tts_transfer(stressed, wave, WAVE_SIZE, wave_consumer, NULL, &tts_params);
              free(stressed);
            }
          else ru_tts_transfer(text, wave, WAVE_SIZE, wave_consumer, NULL, &tts_params);
          s = text;
        }
      else
        {
          text = xrealloc(text, size <<= 1);
          s = text + strlen(text);
        }
    }
  while (input);

  if (db)
    rulexdb_close(db);
  if (slog)
    fclose(slog);

  free(text);
  free(wave);
  return EXIT_SUCCESS;
}
