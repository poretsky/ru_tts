/* Russian TTS library
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_H
#define RU_TTS_H

#include <stdlib.h>

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

typedef int (*ru_tts_callback)(void *buffer, size_t size, void *user_data);

/*
 * Perform TTS transformation for specified text.
 *
 * The first argument points to a zero-terminated string to transfer.
 * This string must contain a Russian text in koi8-r.
 * The next two arguments specify a buffer that will be used
 * by the library for delivering produced wave data
 * chunk by chunk to the consumer specified by the fourth argument.
 * Non-zero return value of the consumer causes immediate speech termination.
 * The next argument points to any additional user data passed to the consumer.
 *
 * Remaining arguments specify TTS parameters:
 *
 * Non-zero value for voice argument means that the alternative voice
 * should be used instead of the default one.
 *
 * The value of rate and pitch should be within [0..250] boundaries.
 * All other values will be reduced to this interval.
 * The last argument is treated as a logical value.
 * If it is zero the produced speech will be monotone.
 */
extern void ru_tts_transfer(const char *text, void *wave_buffer, size_t wave_buffer_size,
                            ru_tts_callback wave_consumer, void *user_data,
                            int voice, int rate, int pitch, int intonation);

END_C_DECLS

#endif
