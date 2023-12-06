# Makefile for ru_tts
#
# Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
# Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
# Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
# Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
#
# SPDX-License-Identifier: MIT
#
# Use command:
#
# make dictionary=skip
#
# to build the synthesizer without dictionary support.
# Otherwise the RuLex dictionary is used.
#
#To make the RuLex library dlopened instead of linked,
# use command:
#
# make dictionary=optional

CC = gcc
CFLAGS ?= -Wall -Wno-unused-result -O2
LDFLAGS ?=
LIBS = -lm
solib_LFLAGS = -Wl,--version-script=ru_tts.vscript

ifeq ($(dictionary),skip)
CFLAGS += -DWITHOUT_DICTIONARY
else ifeq ($(dictionary),optional)
LIBS += -ldl
CFLAGS += -DRULEX_DLL=\"librulexdb.so.0\"
else
LIBS += -lrulexdb
endif

# Installation paths
DESTDIR = 
prefix = /usr/local
bindir = ${prefix}/bin
includedir = ${prefix}/include
libdir = ${prefix}/lib

all:ru_tts

clean:
	libtool --mode=clean rm -f *.lo *.la ru_tts
	rm -rf obj libs *.o

install: ru_tts librutts.la
	libtool --mode=install install -D -p -s librutts.la ${DESTDIR}${libdir}/librutts.la
	libtool --mode=install install -D -p -s ru_tts ${DESTDIR}${bindir}/ru_tts
	install -D -m 0644 -p ru_tts.h ${DESTDIR}${includedir}/ru_tts.h

ru_tts: ru_tts.o librutts.la
	libtool --tag=CC --mode=link ${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

librutts.la: synth.lo sink.lo transcription.lo text2speech.lo \
	 utterance.lo time_planner.lo speechrate_control.lo \
	 intonator.lo soundproducer.lo numerics.lo male.lo female.lo
	libtool --tag=CC --mode=link ${CC} ${LDFLAGS} ${solib_LFLAGS} -o $@ $^ -rpath ${libdir} -version-info 7:4:0

%.lo:%.c
	libtool --tag=CC --mode=compile ${CC} -c ${CFLAGS} ${CPPFLAGS} $<

%.o:%.c
	${CC} -c ${CFLAGS} ${CPPFLAGS} $<

ru_tts.o: ru_tts.c ru_tts.h
synth.lo: synth.c synth.h transcription.h soundscript.h ru_tts.h
numerics.lo: numerics.c numerics.h transcription.h sink.h synth.h ru_tts.h
transcription.lo: transcription.c transcription.h numerics.h sink.h
utterance.lo: utterance.c soundscript.h transcription.h
time_planner.lo: time_planner.c soundscript.h transcription.h
speechrate_control.lo: speechrate_control.c timing.h soundscript.h transcription.h
intonator.lo: intonator.c transcription.h soundscript.h modulation.h
soundproducer.lo: soundproducer.c sink.h soundscript.h voice.h
sink.lo: sink.c sink.h ru_tts.h
male.lo: male.c voice.h
female.lo: female.c voice.h
text2speech.lo: text2speech.c ru_tts.h sink.h transcription.h synth.h timing.h modulation.h
