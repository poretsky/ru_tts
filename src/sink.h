/* sink.h -- General data consuming callback interface
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_SINK_H
#define RU_TTS_SINK_H

#include <stdint.h>
#include <stdlib.h>

#include "ru_tts.h"


/* Sink control block definition */
struct sink_cb
{
  void *buffer;
  size_t buffer_offset;
  size_t bufsize;
  int status;
  void *user_data;
  ru_tts_callback function;
  void (*custom_reset)(struct sink_cb *consumer);
};
typedef struct sink_cb sink_t;


/* Initialize sink control structure */
extern void sink_setup(sink_t *consumer, void *buffer, size_t bufsize, ru_tts_callback function, void *user_data);

/*
 * Reset action.
 *
 * Performs custom reset function if specified.
 * Otherwise simply resets buffer offset.
 */
extern void sink_reset(sink_t *consumer);

/* Pass data to the sink function */
extern void sink_flush(sink_t *consumer);

/* Pass one byte of data */
extern void sink_put(sink_t *consumer, int8_t byte);

/*
 * Pass a block of data.
 *
 * Potentially may cause buffer overflow since this condition
 * is not checked during data transfer, but only afterwards.
 */
extern void sink_write(sink_t *consumer, const uint8_t *block, size_t size);

/* Forget last byte if possible */
extern void sink_back(sink_t *consumer);

/*
 * Replace the last byte in the consumer buffer.
 * If the buffer is empty, acts just like sink_put().
 */
extern void sink_replace(sink_t *consumer, uint8_t byte);

/*
 * Get last byte value from the consumer buffer.
 * Returns -1 if the buffer is empty.
 */
extern int sink_last(sink_t *consumer);

#endif
