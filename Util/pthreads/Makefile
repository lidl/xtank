#
# build a sample pthreads program#
#

PROGS=cond pthreads
CC=gcc
CFLAGS+= -g -Wall #-pg
#LDFLAGS+= -pg
#CLEANFILES+= gmon.out

all: ${PROGS}

clean:
	-rm -f ${PROGS} *.o *.core

pthreads: pthreads.c

cond: cond.c

