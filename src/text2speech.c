/* text2speech.c -- Full TTS transfer
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
#include "timing.h"
#include "modulation.h"
#include "transcription.h"
#include "synth.h"


/* Default TTS configuration */

ru_tts_conf_t ru_tts_config =
  {
    .speech_rate = 100,
    .voice_pitch = 100,
    .intonation = 100,
    .general_gap_factor = 100,
    .comma_gap_factor = 100,
    .dot_gap_factor = 100,
    .semicolon_gap_factor = 100,
    .colon_gap_factor = 100,
    .question_gap_factor = 100,
    .exclamation_gap_factor = 100,
    .intonational_gap_factor = 100,
    .flags = DEC_SEP_POINT | DEC_SEP_COMMA
  };


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
      ttscb_t ttscb;
      sink_t transcription_consumer;

      /* Initialize data structures */
      sink_setup(&(ttscb.wave_consumer), wave_buffer, wave_buffer_size, consumer, user_data);
      sink_setup(&transcription_consumer, transcription_buffer, TRANSCRIPTION_MAXLEN, synth_function, &ttscb);
      ttscb.flags = ru_tts_config.flags;

      /* Adjust speech rate */
      timing_setup(&(ttscb.timing), ru_tts_config.speech_rate, ru_tts_config.general_gap_factor);
      adjust_gaplen(&(ttscb.timing), ',', ru_tts_config.comma_gap_factor);
      adjust_gaplen(&(ttscb.timing), '.', ru_tts_config.dot_gap_factor);
      adjust_gaplen(&(ttscb.timing), ';', ru_tts_config.semicolon_gap_factor);
      adjust_gaplen(&(ttscb.timing), ':', ru_tts_config.colon_gap_factor);
      adjust_gaplen(&(ttscb.timing), '?', ru_tts_config.question_gap_factor);
      adjust_gaplen(&(ttscb.timing), '!', ru_tts_config.exclamation_gap_factor);
      adjust_gaplen(&(ttscb.timing), '-', ru_tts_config.intonational_gap_factor);

      /* Adjust voice pitch and intonation */
      modulation_setup(&(ttscb.modulation), ru_tts_config.voice_pitch, ru_tts_config.intonation);

      /* Process text */
      process_text(text, &transcription_consumer);
      free(transcription_buffer);
    }
}
