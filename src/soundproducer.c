/* soundproducer.c -- Output sound stream generator
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>

#include "soundscript.h"
#include "voice.h"
#include "sink.h"


/* Local data */

/* Control data used to generate fully synthetic sounds */
static const int16_t synth_ctrl_data[][2] =
  {
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0x930C, 0 },
    { 0x0CF74, 1 },
    { 2, 0x8002 },
    { 0x930C, 1 },
    { 2, 1 },
    { 2, 0x8003 },
    { 0x930C, 0 },
    { 0x0CF74, 1 },
    { 0x308C, 1 },
    { 0x0B8B, 2 },
    { 0x502E, 1 },
    { 0x66F0, 1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 },
    { 0, -1 }
  };


/* Local subroutines */

/* Evaluate speech parameters according to the specified stage */
static int16_t eval(icb_t *icb)
{
  int16_t res = (icb->stretch >> 1);
  if (!(--icb->count))
    {
      icb->stretch += icb->delta;
      icb->count = icb->period;
    }
  return res;
}

/*
 * Make a silence of specified length.
 * Returns length.
 */
static uint16_t silence(sink_t *consumer, uint16_t length)
{
  uint16_t i;
  for (i = 0; i < length; i++)
    sink_put(consumer, 0);
  return length;
}

/*
 * Make fading from specified sample index in the voice data.
 * Returns number of generated samples.
 */
static uint16_t fading(sink_t *consumer, const voice_t *voice, uint16_t sidx)
{
  uint16_t i;
  int8_t sample = voice->samples[sidx - 1];
  for (i = 0; i < 3; i++)
    {
      sample >>= 1;
      sink_put(consumer, sample);
    }
  return 3;
}


/* Global entry point */

/* Generate sound stream and feed it to the specified sink */
void make_sound(soundscript_t *script, sink_t *consumer)
{
  int i;

  sink_put(consumer, 0);
  for (i = 0; (i < script->length) && !consumer->status; i++)
    {
      int16_t l = script->sounds[i].duration;
      uint16_t j = ((uint16_t)(script->sounds[i].id)) & 0xFF;
      uint16_t k;
      if (j >= 169)
        {
          /* Fully synthetic sounds that are not voice dependent */
          int16_t bx;
          int16_t cx;
          j -= 169;
          bx = synth_ctrl_data[j][0];
          cx = synth_ctrl_data[j][1];
          if (cx != -1)
            {
              uint16_t ax = 205;
              int16_t sample_shift = (cx & 0xFF) + 8;
              int16_t var1 = 0;
              int16_t var2 = 0;
              int16_t var3 = 0;
              for (k = 0; k <= l; k++)
                {
                  int16_t si;
                  int16_t tmp = ax & 0x2D;
                  tmp ^= tmp >> 4;
                  tmp &= 0x0F;
                  if ((0x6996 >> tmp) & 0x01)
                    ax |= 0x8000;
                  ax >>= 1;
                  tmp = ax;
                  ax >>= 2;
                  var3 >>= 1;
                  var3 += var3 >> 2;
                  if (cx >= 0)
                    var3 += var3 >> 2;
                  si = var3;
                  var3 = (var2 << 1) - var1;
                  var1 = ax;
                  ax = (uint16_t)((((int32_t)var3) * ((int32_t)bx)) >> 15);
                  ax += var1 - si;
                  var3 = var2;
                  var2 = ax;
                  sink_put(consumer, (int8_t)(var2 >> sample_shift));
                  ax = tmp;
                }
            }
          else silence(consumer, l);
        }
      else if (l) /* Non-zero sound length check */
        {
          int16_t ax = 0;
          uint16_t sidx = script->voice->sound_offsets[j];
          uint16_t scnt = script->voice->sound_lengths[j];
          uint8_t stage = script->sounds[i].stage;
          if (scnt > VOICE_THRESHOLD)
            {
              /* Simply copy prepared pattern from the voice data */
              do sink_put(consumer, script->voice->samples[sidx++]);
              while ((--scnt) && (--l));
            }
          else if (j >= 132)
            {
              /* Make a transition of a prepared pattern */
              while (l > ax)
                {
                  k = script->icb[stage].stretch;
                  do
                    {
                      sink_put(consumer, script->voice->samples[sidx++]);
                      l--;
                    }
                  while ((--k) && (--scnt));
                  if (k)
                    l -= silence(consumer, k);
                  else if (scnt > 1)
                    l -= fading(consumer, script->voice, sidx);
                  ax = eval(&(script->icb[stage]));
                  sidx = script->voice->sound_offsets[j];
                  scnt = script->voice->sound_lengths[j];
                }
            }
          else
            {
              /* Mixed and transitional sounds */
              int16_t dx = 0;
              while (l >= ax)
                {
                  uint16_t next_pattern_offset;
                  k = script->icb[stage].stretch;
                  j = ((uint16_t)(script->sounds[i + 1].id)) & 0xFF;
                  next_pattern_offset = script->voice->sound_offsets[j + 1];
                  j = script->voice->sound_offsets[j];
                  sink_put(consumer, 0);
                  ax = (int16_t)(script->voice->samples[j]);
                  do
                    {
                      ax -= (int16_t)(script->voice->samples[sidx]);
                      ax = (int16_t)(((int32_t)ax) * ((int32_t)(dx++)) / ((int32_t)l));
                      ax += (int16_t)(script->voice->samples[sidx++]);
                      sink_put(consumer, (int8_t)ax);
                      ax = ((++j) < next_pattern_offset) ? ((int16_t)(script->voice->samples[j])) : 0;
                    }
                  while ((--k) && (--scnt));
                  if (k)
                    dx += silence(consumer, k);
                  else if (scnt > 1)
                    dx += fading(consumer, script->voice, sidx);
                  ax = dx + eval(&(script->icb[stage]));
                  j = ((uint16_t)(script->sounds[i].id)) & 0xFF;
                  sidx = script->voice->sound_offsets[j];
                  scnt = script->voice->sound_lengths[j];
                }
            }
        }
    }

  /* Flush output buffer */
  sink_flush(consumer);
}
