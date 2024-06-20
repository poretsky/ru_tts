## Process this file with automake to produce Makefile.in

bin_PROGRAMS = ru_tts
lib_LTLIBRARIES = librutts.la
include_HEADERS = ru_tts.h

AM_CFLAGS = -Wall -Wno-unused-result -O2

ru_tts_SOURCES = ru_tts.c
ru_tts_LDADD = librutts.la

librutts_la_LDFLAGS = -version-info 7:4:0

if HAVE_VSCRIPT
librutts_la_LDFLAGS += $(VSCRIPT_LDFLAGS),ru_tts.vscript
endif

librutts_la_SOURCES = synth.c sink.c transcription.c text2speech.c \
	utterance.c time_planner.c speechrate_control.c \
	intonator.c soundproducer.c numerics.c male.c female.c

EXTRA_DIST = ru_tts.vscript modulation.h numerics.h soundscript.h \
	synth.h sink.h timing.h transcription.h voice.h
MAINTAINERCLEANFILES = Makefile.in config.h.in config.h.in~
