/* sink.c -- Interactions with general data consuming callback
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

#include "ru_tts.h"
#include "sink.h"


/* Initialize sink control structure */
void sink_setup(sink_t *consumer, void *buffer, size_t bufsize, ru_tts_callback function, void *user_data)
{
  consumer->buffer = buffer;
  consumer->buffer_offset = 0;
  consumer->bufsize = bufsize;
  consumer->status = 0;
  consumer->user_data = user_data;
  consumer->function = function;
  consumer->custom_reset = NULL;
}

/*
 * Reset action.
 *
 * Performs custom reset function if specified.
 * Otherwise simply resets buffer offset.
 */
void sink_reset(sink_t *consumer)
{
  if (consumer->custom_reset)
    consumer->custom_reset(consumer);
  else consumer->buffer_offset = 0;
}

/* Pass data to the sink function */
void sink_flush(sink_t *consumer)
{
  if (consumer->function && consumer->buffer && consumer->buffer_offset)
    consumer->status |= consumer->function(consumer->buffer, consumer->buffer_offset, consumer->user_data);
  sink_reset(consumer);
}

/* Pass one byte of data */
void sink_put(sink_t *consumer, int8_t byte)
{
  ((int8_t *)(consumer->buffer))[consumer->buffer_offset++] = byte;
  if (consumer->buffer_offset >= consumer->bufsize)
    sink_flush(consumer);
}

/*
 * Pass a block of data.
 *
 * Potentially may cause buffer overflow since this condition
 * is not checked during data transfer, but only afterwards.
 */
void sink_write(sink_t *consumer, const uint8_t *block, size_t size)
{
  memcpy((uint8_t *)consumer->buffer + consumer->buffer_offset, block, size);
  consumer->buffer_offset += size;
  if (consumer->buffer_offset >= consumer->bufsize)
    sink_flush(consumer);
}

/* Forget last byte if possible */
void sink_back(sink_t *consumer)
{
  if (consumer->buffer_offset)
    consumer->buffer_offset--;
}

/*
 * Replace the last byte in the consumer buffer.
 * If the buffer is empty, acts just like sink_put().
 */
void sink_replace(sink_t *consumer, uint8_t byte)
{
  sink_back(consumer);
  sink_put(consumer, byte);
}

/*
 * Get last byte value from the consumer buffer.
 * Returns -1 if the buffer is empty.
 */
int sink_last(sink_t *consumer)
{
  return (consumer->buffer_offset) ? ((uint8_t *)(consumer->buffer))[consumer->buffer_offset - 1] : -1;
}
