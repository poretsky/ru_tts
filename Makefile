# Makefile for compiling and installing ru_tts
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
#
# To install ru_tts use command:
#
# make install

# Export all variables
export

# Installation paths
DESTDIR = 
prefix = /usr/local
bindir = ${prefix}/bin
docdir = ${prefix}/share/doc/ru-tts
mandir = ${prefix}/share/man

all:
	$(MAKE) -e -C src all

install:
	$(MAKE) -e -C src install
	install -m 0755 -p -D -t ${DESTDIR}${bindir} ru_speak
	install -d ${DESTDIR}${docdir}
	install -m 0644 -p README README.ru ${DESTDIR}${docdir}
	install -d ${DESTDIR}${mandir}/man1
	install -m 0644 -p manpages/*.1 ${DESTDIR}${mandir}/man1
	install -d ${DESTDIR}${mandir}/man3
	install -m 0644 -p manpages/*.3 ${DESTDIR}${mandir}/man3

clean:
	$(MAKE) -C src clean
