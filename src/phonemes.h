/* phonemes.h -- phoncodes
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RU_TTS_PHONEMES_H
#define RU_TTS_PHONEMES_H

typedef enum 
  {
    PH_U = 0,  /* [у] */
    PH_O = 1,  /* [о] */
    PH_A = 2,  /* [а] */
    PH_E = 3,  /* [э] */
    PH_Y = 4,  /* [ы] */
    PH_I = 5,  /* [и] */
    PH_V = 6,  /* [в] */
    PH_Z = 7,  /* [з] */
    PH_R = 8,  /* [р] */
    PH_ZH = 9,  /* [ж] */
    PH_J = 10, /* [й] */
    PH_V_ = 11, /* [в'] */
    PH_Z_ = 12, /* [з'] */
    PH_R_ = 13, /* [р'] */
    PH_L = 14, /* [л] */
    PH_M = 15, /* [м] */
    PH_N = 16, /* [н] */
    PH_L_ = 17, /* [л'] */
    PH_M_ = 18, /* [м'] */
    PH_N_ = 19, /* [н'] */
    PH_B = 20, /* [б] */
    PH_D = 21, /* [д] */
    PH_G = 22, /* [г] */
    PH_B_ = 23, /* [б'] */
    PH_D_ = 24, /* [д'] */
    PH_G_ = 25, /* [г'] */
    PH_P = 26, /* [п] */
    PH_T = 27, /* [т] */
    PH_K = 28, /* [к] */
    PH_P_ = 29, /* [п'] */
    PH_T_ = 30, /* [т'] */
    PH_K_ = 31, /* [к'] */
    PH_C = 32, /* [ц] */
    PH_CH = 33, /* [ч] */
    PH_F = 34, /* [ф] */
    PH_S = 35, /* [с] */
    PH_SH = 36, /* [ш] */
    PH_F_ = 37, /* [ф'] */
    PH_S_ = 38, /* [с'] */
    PH_SH_ = 39, /* [щ'] */
    PH_X = 40, /* [х] */
    PH_X_ = 41, /* [х'] */
    PH_TERMINATOR = 42, /* '#' */
    PH_SPACE = 43, /* ' ' */
    PH_COMMA = 44, /* ',' */
    PH_PERIOD = 45, /* '.' */
    PH_SEMICOLON = 46, /* ';' */
    PH_COLON = 47, /* ':' */
    PH_QUESTION = 48, /* '?' */
    PH_EXCLAMATION = 49, /* '!' */
    PH_OPEN_BRACKET = 50, /* '(' */
    PH_CLOSE_BRACKET = 51, /* ')' */
    PH_MINUS = 52, /* '-' */
    PH_PRIMARY_STRESS = 53, /* '+' */
    PH_SECONDARY_STRESS = 54  /* '=' */
  } Phoneme;

#endif 
