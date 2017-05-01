#/*-
# * Copyright (c) 2016, 2017 Kurt Lidl
# * All rights reserved.
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met:
# * 1. Redistributions of source code must retain the above copyright
# *    notice, this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# *    notice, this list of conditions and the following disclaimer in the
# *    documentation and/or other materials provided with the distribution.
# *
# * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# * SUCH DAMAGE.
# */

#
# GNUmakefile for the 'xtank' program
#

.CURDIR= .
PREFIX=/usr/local

# When this was built using Imake, the Display, Programs and Threads
# directories were built into different libraries.  Here, we're just
# going to build them all into this directory and forgo linking the
# code into libraries.

VPATH=	${.CURDIR}/Display:${.CURDIR}/Programs:${.CURDIR}/Threads

ifneq ("$(wildcard /opt/X11/include)","")
CFLAGS0+= -I/opt/X11/include -D_XOPEN_SOURCE
LDFLAGS+= -L/opt/X11/lib
endif
ifneq ("$(wildcard /usr/local/include/X11)","")
CFLAGS0+= -I/usr/local/include
LDFLAGS+= -L/usr/local/lib
endif

include ${.CURDIR}/Makefile.inc1

ifeq ($(shell uname),Linux)
LDLIBS+= -ldl
endif

LDLIBS+= -lX11 -lm

OBJS:=	$(patsubst %.c,%.o,$(SRCS))

all:	${PROG}

install: ${PROG}
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -s ${PROG} ${DESTDIR}${PREFIX}/bin

${PROG}: ${OBJS}
	${LINK.c} $^ ${LOADLIBES} ${LDLIBS} -o $@

include ${.CURDIR}/Makefile.inc2

clean:
	rm -f ${PROG} ${OBJS} ${CLEANFILES} *.core

.PHONY: all clean install
