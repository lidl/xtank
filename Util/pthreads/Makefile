#
# build a sample pthreads program#
#

PROG=pthreads
NOMAN=
CC=gcc
LDCC=${CC}
CFLAGS+= -g -pg -Wall
LDFLAGS+= -pg
CLEANFILES+= gmon.out

.include <bsd.prog.mk>
