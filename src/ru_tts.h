/* Russian TTS library
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_H
#define RU_TTS_H

#include <stdlib.h>


#if defined(_WIN32) || defined(_WIN64)
#define RUTTS_EXPORT __declspec(dllexport)
#else
#define RUTTS_EXPORT
#endif

/* TTS control flags */
#define DEC_SEP_POINT 1 /* Use point as a decimal separator */
#define DEC_SEP_COMMA 2 /* Use comma as a decimal separator */
#define USE_ALTERNATIVE_VOICE 4


/* BEGIN_C_DECLS should be used at the beginning of C declarations,
   so that C++ compilers don't mangle their names.  Use END_C_DECLS at
   the end of C declarations. */
#undef BEGIN_C_DECLS
#undef END_C_DECLS
#ifdef __cplusplus
# define BEGIN_C_DECLS extern "C" {
# define END_C_DECLS }
#else
# define BEGIN_C_DECLS /* empty */
# define END_C_DECLS /* empty */
#endif

BEGIN_C_DECLS

/* Callback function to utilize generated sound */
typedef int (*ru_tts_callback)(void *buffer, size_t size, void *user_data);

/* Speech parameters */
typedef struct
{
  int speech_rate; /* Reasonable value range is [20..500]. */
  int voice_pitch; /* Reasonable value range is [50..300]. */
  int intonation; /* Reasonable value range is [0..140].
                     Greater values imply more expressive speech. */

  /* Interclause gap durations adjustment */
  int general_gap_factor; /* Reasonable value range is [0..x].
                             Upper boundary depends on speech rate.
                             At the minimum speech rate it is 1.25.
                             Quicker speech allows to use greater values. */
  int comma_gap_factor; /* Reasonable value range is [0..750] */
  int dot_gap_factor; /* Reasonable value range is [0..500] */
  int semicolon_gap_factor; /* Reasonable value range is [0..600] */
  int colon_gap_factor; /* Reasonable value range is [0..600] */
  int question_gap_factor; /* Reasonable value range is [0..375] */
  int exclamation_gap_factor; /* Reasonable value range is [0..300] */
  int intonational_gap_factor; /* Reasonable value range is [0..1000] */

  /* Combination of TTS control flags */
  int flags;
} ru_tts_conf_t;

/*
 * Initialize ru_tts configuration structure with the default values.
 */
extern RUTTS_EXPORT void ru_tts_config_init(ru_tts_conf_t *config);

/*
 * Perform TTS transformation for specified text.
 *
 * The first argument points to a configuration structure.
 * The second argument points to a zero-terminated string to transfer.
 * This string must contain a Russian text in koi8-r.
 * The next two arguments specify a buffer that will be used
 * by the library for delivering produced wave data
 * chunk by chunk to the consumer specified by the fourth argument.
 * Non-zero return value of the consumer causes immediate speech termination.
 * The last argument points to any additional user data passed to the consumer.
 */
extern RUTTS_EXPORT void ru_tts_transfer(const ru_tts_conf_t *config,
                                         const char *text, void *wave_buffer, size_t wave_buffer_size,
                                         ru_tts_callback wave_consumer, void *user_data);

END_C_DECLS

#endif
