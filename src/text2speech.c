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

#include "ru_tts.h"
#include "sink.h"
#include "transcription.h"
#include "synth.h"


/* Default TTS configuration */

ru_tts_conf_t ru_tts_config =
  {
    .speech_rate = 100,
    .voice_pitch = 100,
    .gap_factor = 100,
    .intonation = 100,
    .flags = DEC_SEP_POINT | DEC_SEP_COMMA
  };


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
 * The last argument points to a structure containing TTS parameters.
 */
void ru_tts_transfer(const char *text, void *wave_buffer, size_t wave_buffer_size,
                     ru_tts_callback consumer, void *user_data)
{
  uint8_t *transcription_buffer = malloc(TRANSCRIPTION_BUFFER_SIZE);

  if (transcription_buffer)
    {
      int gaplen = (900 - (ru_tts_config.speech_rate << 2)) >> 2;
      ttscb_t ttscb;
      sink_t transcription_consumer;

      /* Initialize data structures */
      sink_setup(&(ttscb.wave_consumer), wave_buffer, wave_buffer_size, consumer, user_data);
      sink_setup(&transcription_consumer, transcription_buffer, TRANSCRIPTION_MAXLEN, synth_function, &ttscb);
      transcription_consumer.custom_reset = transcription_init;
      ttscb.flags = ru_tts_config.flags;

      /* Adjust speech rate */
      if (ru_tts_config.speech_rate < 38)
        {
          ttscb.timing.rate_factor = 125;
          ttscb.timing.stretch = 10;
          gaplen = 188;
        }
      else if (ru_tts_config.speech_rate > 225)
        {
          ttscb.timing.rate_factor = 0;
          ttscb.timing.stretch = 4;
          gaplen = 0;
        }
      else if (ru_tts_config.speech_rate < 100)
        {
          ttscb.timing.rate_factor = 199 - (ru_tts_config.speech_rate << 1);
          ttscb.timing.stretch = 10;
        }
      else
        {
          ttscb.timing.rate_factor = 225 - ru_tts_config.speech_rate;
          ttscb.timing.stretch = 4;
        }
      gaplen += 12;
      gaplen *= ru_tts_config.gap_factor;
      gaplen /= 100;
      if (gaplen < 0)
        ttscb.timing.gaplen = 0;
      else if (gaplen > 250)
        ttscb.timing.gaplen = 250;
      else ttscb.timing.gaplen = (uint8_t)gaplen ;

      /* Adjust voice pitch */
      if (ru_tts_config.voice_pitch < 50)
        ttscb.mintone = 50;
      else if (ru_tts_config.voice_pitch > 300)
        ttscb.mintone = 300;
      else ttscb.mintone = (uint16_t) ru_tts_config.voice_pitch;
      ttscb.maxtone = ttscb.mintone;

      /* Adjust intonation */
      if (ru_tts_config.intonation > 0)
        ttscb.maxtone += (ru_tts_config.intonation < 140) ?
          (((ttscb.mintone >> 1) + 25) * ru_tts_config.intonation / 100) :
          (ttscb.mintone * 7 / 10 + 35);

      /* Respect voice */
      if (ru_tts_config.flags & USE_ALTERNATIVE_VOICE)
        {
          ttscb.mintone <<= 1;
          ttscb.maxtone <<= 1;
        }

      /* Process text */
      process_text(text, &transcription_consumer);
      free(transcription_buffer);
    }
}
