/* text2speech.c -- Full TTS transfer
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
#include <stdio.h>

#include "ru_tts.h"
#include "sink.h"
#include "transcription.h"
#include "synth.h"


/* Transcription consumer callback functions */

static void transcription_init(sink_t *consumer)
{
  uint8_t *buffer = consumer->buffer;
  memset(buffer, 43, TRANSCRIPTION_BUFFER_SIZE);
  consumer->buffer_offset = TRANSCRIPTION_START;
}

static int synth_function(void *buffer, size_t length, void *user_data)
{
  ttscb_t *ttscb = user_data;
  if (length > TRANSCRIPTION_START)
    {
      if (ttscb->transcription_state.flags & CLAUSE_DONE)
        ttscb->transcription_state.flags &= ~CLAUSE_DONE;
      else
        {
          ((uint8_t *)buffer)[length] = 44;
          ttscb->transcription_state.clause_type = 0;
        }
      synth(buffer, ttscb);
    }
  return ttscb->wave_consumer.status;
}


/* Common entry point */

/*
 * Perform TTS transformation for specified text.
 *
 * The first argument points to a zero-terminated string to transfer.
 * This string must contain a Russian text in koi8-r.
 * The next two arguments specify a buffer that will be used
 * by the library for delivering produced wave data
 * chunk by chunk to the consumer specified by the fourth argument.
 * The next argument points to any additional user data passed to the consumer.
 *
 * Remaining arguments specify TTS parameters:
 *
 * Non-zero value for voice argument means that the alternative voice
 * should be used instead of the default one.
 *
 * The value of rate and pitch should be within [0..250] boundaries.
 * All other values will be reduced to this interval.
 *
 * The gaplen value specifies relative interphrase gap duration.
 * It must be within [0..100] range. All other values
 * will be reduced to this interval.
 *
 * The last argument is treated as a logical value.
 * If it is zero the produced speech will be monotone.
 */
void ru_tts_transfer(const char *text, void *wave_buffer, size_t wave_buffer_size,
                     ru_tts_callback consumer, void *user_data,
                     int voice, int rate, int pitch, int gap_factor, int intonation)
{
  ttscb_t ttscb;
  sink_t transcription_consumer;
  uint8_t *transcription_buffer = malloc(TRANSCRIPTION_BUFFER_SIZE);

  if (transcription_buffer)
    {
      int gaplen = (1000 - (rate << 2)) >> 2;

      sink_setup(&(ttscb.wave_consumer), wave_buffer, wave_buffer_size, consumer, user_data);
      sink_setup(&transcription_consumer, transcription_buffer, TRANSCRIPTION_MAXLEN, synth_function, &ttscb);
      transcription_consumer.custom_reset = transcription_init;

      /* Choose voice */
      ttscb.voice = voice ? &female : &male;

      /* Adjust speech rate */
      if (rate < 0)
        {
          ttscb.rate_factor = 125;
          ttscb.stretch = 10;
          gaplen = 250;
        }
      else if (rate > 250)
        {
          ttscb.rate_factor = 0;
          ttscb.stretch = 4;
          gaplen = 0;
        }
      else if (rate < 125)
        {
          ttscb.rate_factor = 125 - rate;
          ttscb.stretch = 10;
        }
      else
        {
          ttscb.rate_factor = 250 - rate;
          ttscb.stretch = 4;
        }
      gaplen *= gap_factor;
      gaplen += 50;
      gaplen /= 100;
      if (gaplen < 0)
        ttscb.gaplen = 0;
      else if (gaplen > 250)
        ttscb.gaplen = 250;
      else ttscb.gaplen = (uint8_t)gaplen ;

      /* Adjust voice pitch */
      if (pitch < 0)
        ttscb.mintone = 60;
      else if (pitch > 250)
        ttscb.mintone = 310;
      else ttscb.mintone = (short int)(pitch + 60);

      /* Adjust intonation */
      ttscb.maxtone = intonation ? (ttscb.mintone * 3 / 2 + 30) : ttscb.mintone;

      /* Process text */
      process_text(text, &transcription_consumer);
      free(transcription_buffer);
    }
}
