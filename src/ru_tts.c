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

/* Acceptable character codes */
static const char symbols[] =
  {
    '+', '=',
    0xC1, 0xC2, 0xD7, /* а, б, в, */
    0xC7, 0xC4, 0xC5, /* г, д, е, */
    0xA3, 0xD6, 0xDA, /* ё, ж, з, */
    0xC9, 0xCA, 0xCB, /* и, й, к, */
    0xCC, 0xCD, 0xCE, /* л, м, н, */
    0xCF, 0xD0, 0xD2, /* о, п, р, */
    0xD3, 0xD4, 0xD5, /* с, т, у, */
    0xC6, 0xC8, 0xC3, /* ф, х, ц, */
    0xDE, 0xDB, 0xDD, /* ч, ш, щ, */
    0xDF, 0xD9, 0xD8, /* ъ, ы, ь, */
    0xDC, 0xC0, 0xD1, /* э, ю, я, */
    0
  };

static const char *help_msg =
  "Speech control options:\n"
  "-r value -- Speech rate factor.\n"
  "-p value -- Voice pitch factor.\n"
  "-e value -- Generated speech emotionality factor.\n"
  "-g value -- Interclause gaps duration factor.\n"
  "-a -- Use alternative (female) voice.\n\n"

  "Numbers treatment control options:\n"
  "-d. -- Only point should be treated as decimal separator.\n"
  "-d, -- Only comma should be treated as decimal separator.\n"
  "-d- -- Disable decimal separators treating.\n\n"

  "Other options:\n"
  "-s path -- Pronunciation dictionary location.\n"
  "-l path -- Log unknown words in specified file\n"
  "           (does matter only in conjunction with pronunciation dictionary).\n"
  "-h -- Get this help.\n\n"

  "All numeric parameters must be positive numbers. Default value is 1.0.\n\n"

  "A value for -g option may be prepended by one of the following\n"
  "punctuations: ',', '.', ';', ':', '?', '!'. In this case the factor\n"
  "will be applied only to the gaps implied by that punctuation.\n"
  "If the value is prepended by symbol '-', it will be used for\n"
  "intonational gaps not caused by any punctuation.\n"
  "Otherwise all interclause gaps will be affected.\n"
  "Use this option several times to adjust various gaps.\n";

static const char *clause_separators = ",.;:?!-";
static const char *charset = "ru_RU.koi8r";
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

static int getval(const char *s)
{
  char *t;
  double value = strtod(s, &t);
  if ((s == t) || (value < 0.0))
    {
      fprintf(stderr, "Illegal option value \"%s\"\n", s);
      fprintf(stderr, "Must be a positive number\n");
      exit(EXIT_FAILURE);
    }
  return rint(value * 100.0);
}

static void usage(const char* name)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "%s [options]\n\n", name);
  fprintf(stderr, "%s\n", help_msg);
}

int main(int argc, char **argv)
{
  size_t size = 64;
  char c, *s, *text, *input;
  void *wave;
  FILE *slog = NULL;
  RULEXDB *db = NULL;

  while ((c = getopt(argc, argv, "s:l:r:p:g:e:d:ah")) != -1)
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
          case 'a':
            ru_tts_config.flags |= USE_ALTERNATIVE_VOICE;
            break;
          case 'r':
            ru_tts_config.speech_rate = getval(optarg);
            break;
          case 'p':
            ru_tts_config.voice_pitch = getval(optarg);
            break;
          case 'g':
            if (strchr(clause_separators, *optarg))
              switch (*optarg)
                {
                case ',':
                  ru_tts_config.comma_gap_factor = getval(optarg + 1);
                  break;
                case '.':
                  ru_tts_config.dot_gap_factor = getval(optarg + 1);
                  break;
                case ';':
                  ru_tts_config.semicolon_gap_factor = getval(optarg + 1);
                  break;
                case ':':
                  ru_tts_config.colon_gap_factor = getval(optarg + 1);
                  break;
                case '?':
                  ru_tts_config.question_gap_factor = getval(optarg + 1);
                  break;
                case '!':
                  ru_tts_config.exclamation_gap_factor = getval(optarg + 1);
                  break;
                case '-':
                  ru_tts_config.intonational_gap_factor = getval(optarg + 1);
                  break;
                default:
                  break;
                }
            else ru_tts_config.general_gap_factor = getval(optarg);
            break;
          case 'e':
            ru_tts_config.intonation = getval(optarg);
            break;
          case 'd':
            ru_tts_config.flags &= ~(DEC_SEP_POINT | DEC_SEP_COMMA);
            switch (*optarg)
              {
              case '.':
                ru_tts_config.flags |= DEC_SEP_POINT;
                break;
              case ',':
                ru_tts_config.flags |= DEC_SEP_COMMA;
              case '-':
                break;
              default:
                fprintf(stderr, "Invalid option \"-d%s\"\n\n", optarg);
                usage(argv[0]);
                return EXIT_FAILURE;
              }
            break;
          case 'h':
            usage(argv[0]);
            return EXIT_SUCCESS;
          default:
            fprintf(stderr, "\n");
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
              ru_tts_transfer(stressed, wave, WAVE_SIZE, wave_consumer, NULL);
              free(stressed);
            }
          else ru_tts_transfer(text, wave, WAVE_SIZE, wave_consumer, NULL);
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
