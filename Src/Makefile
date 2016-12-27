#
# Makefile for the 'xtank' program
#

# When this was built using Imake, the Display, Programs and Threads
# directories were built into different libraries.  Here, we're just
# going to build them all into this directory and forgo linking the
# code into libraries.

.PATH:	${.CURDIR}/Display ${.CURDIR}/Programs ${.CURDIR}/Threads

PROG=	xtank
MK_MAN=no

CFLAGS0+= -I${.CURDIR} -I${.CURDIR}/Include \
	-I/usr/local/include -O0 -g

CFLAGS0+= -DX11 -DUNIX -DS1024x864 -DKEYPAD_DETECT -DSTINGY_REDRAW \
	-DBATCH_LINES -DBATCH_POINTS -DBATCH_COLOR_LINES \
	-DCODE_SIZE=0x20000 -DSTACK_SIZE=0x8000 \
	-DNEW_LIMITS -DSLOWER

CFLAGS0+= -DDYNALOAD -DMULTI_SYNC

CFLAGS1+= -DXTANK_DIR="\"${.CURDIR}/..\""

DEFINES=-DALLDEFINES="\"${CFLAGS0}\""

CFLAGS+=${DEFINES} ${CFLAGS0} ${CFLAGS1}

LDADD+= -L/usr/local/lib -lX11 -lm
.if exists(/usr/local/lib/libefence.a)
LDADD+= -lefence
.endif

SRCS=	actions.c animate.c box.c camo.c collision.c cosell.c \
	disc.c display.c escher.c explosion.c file.c game.c gr.c \
	highlib.c hit.c hud.c icounter.c init.c input.c interface.c \
	intersect.c lowlib.c main.c malloc.c mapper.c maze.c \
	mdesign.c menu.c message.c newconsole.c newfile.c newradar.c \
	players.c program.c radar.c repair.c scroll.c setup.c \
	setups.c status.c sysdep.c unix.c update.c util.c vdesign.c \
	vehicle.c version.c lex.zz.c
SRCS+=	XMultiSync.c graphics.c objects.c three-d.c
SRCS+=	Bootlegger.c Buddy.c Dio.c Diophantine.c Flipper.c Guard.c \
	Pzkw_I.c RacerX.c artful.c drone.c dum_maze.c eliza.c gnat.c \
	hud3.c kamikaze.c mmtf.c rdfbot.c roadrunner.c runner.c \
	spot.c tagman.c warrior.c

lex.zz.c: vload.l
	${LEX} -Pzz ${.CURDIR}/vload.l

version.c: ${.CURDIR}/../Bin/newversion.sh
	sh ${.CURDIR}/../Bin/newversion.sh

cscope:
	(cd ${.CURDIR} && find . -name \*\.\[chly] -a -type f >cscope.files)
	(cd ${.CURDIR} && cscope -bRk)
	@echo "Now run 'cscope -d'"

CLEANFILES+= setups.c version.c lex.zz.c \
	${.CURDIR}/cscope.files  ${.CURDIR}/cscope.out

.include <bsd.prog.mk>
