#
# This makefile describes how to make xtank on the following machines
#
#   DEC VAX
#   IBM RT
#   Sun-3 and Sun-4
#   HP9000 series 800
#
#   sgi (add -D_BSD_SIGNALS to CFLAGS & CCNOPT)
#
# Copyright 1988 by Terry Donahue
#

XOBJS = XMultiSync.o
XSRCS = XMultiSync.c
XINCL = Xlibint.h Xlibos.h
HP9K8OBJS = flush_hp9k8.o
VAXOBJS = vaxsj.o
CFLAGS = -g
# turn optimization off (for init.c)
CCNOPT = -g

OBJS =	main.o interface.o file.o setup.o init.o animate.o game.o program.o \
	update.o box.o collision.o hit.o display.o 3d.o gr.o graphics.o \
	thread.o objects.o input.o actions.o maze.o console.o mapper.o \
	radar.o lowlib.o highlib.o status.o unix.o util.o intersect.o \
	escher.o drone.o warrior.o shooter.o eliza.o Buddy.o kamikaze.o \
	Flipper.o Dio.o Diophantine.o artful.o spot.o vdesign.o mdesign.o menu.o \
	hud.o \
	icounter.o message.o cosell.o repair.o $(XOBJS) malloc.o scroll.o

SRCS =	main.c interface.c file.c setup.c init.c animate.c game.c program.c \
	update.c box.c collision.c hit.c display.c 3d.c gr.c graphics.c \
	thread.c objects.c input.c actions.c maze.c console.c mapper.c \
	radar.c lowlib.c highlib.c status.c unix.c util.c intersect.c \
	escher.c drone.c warrior.c shooter.c eliza.c Buddy.c vdesign.c \
	mdesign.c menu.c icounter.c message.c cosell.c repair.c Flipper.c \
	Dio.c Diophantine.c artful.c spot.c scroll.c hud.c

PICS =	Objects/lightc.obj \
	Objects/hexo.obj \
	Objects/spider.obj \
	Objects/psycho.obj \
	Objects/tornado.obj \
	Objects/marauder.obj \
	Objects/tiger.obj \
	Objects/rhino.obj \
	Objects/medusa.obj \
	Objects/malice.obj \
\
	Objects/shock.obj \
	Objects/gleam.obj \
	Objects/tink.obj \
	Objects/soft.obj \
	Objects/circle.obj \
	Objects/fiery.obj \
	Objects/double.obj \
	Objects/exhaust.obj \
\
	Objects/turret_sm.obj \
	Objects/all_bullets.obj \
\
	Objects/anim_landmarks.obj \
	Objects/map_landmarks.obj \
	Objects/design_landmarks.obj \
\
	Objects/xtank.obj \
	Objects/team.obj \
	Objects/terp.obj

INCL =  disc.h malloc.h structs.h vstructs.h gr.h map.h sysdep.h x11.h box.h \
	graphics.h mdesign.h terminal.h xtank.h buddy.h icounter.h menu.h \
	thread.h xtanklib.h bullet.h interface.h message.h types.h common.h \
	limits.h object.h vdesign.h config.h linetext.h screen.h vehicle.h \
	cosell.h loc.h scroll.h vehicle_parts.h

LIB = -lm -lX11 -llwp
PROGRAM = xtank


xtank: 
	make xtank.`arch`

release: clean
	rm -f xtank.sun4 xtank.sun3
	cd ../.. ; tar cvf /tmp/xtank.tar ./xtank
	compress /tmp/xtank.tar
	cd ../.. ; mv /tmp/xtank.tar.Z .

xtank.sun3 sun3: $(OBJS) grpobj
	$(CC) -Bstatic /usr/lib/debug/malloc.o $(OBJS) $(CFLAGS) -o $(PROGRAM).sun3 $(LIB)

xtank.sun4 sun4: $(OBJS) grpobj
	$(CC) -Bstatic /usr/lib/debug/malloc.o $(OBJS) $(CFLAGS) -o $(PROGRAM).sun4  $(LIB)

xtank.sgi sgi: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(PROGRAM).sgi -lm43 -lX11 -lbsd -lsun -lm

xtank.rt rt: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(PROGRAM).rt $(LIB)

xtank.vax vax: $(OBJS) $(VAXOBJS)
	$(CC) $(OBJS) $(VAXOBJS) $(CFLAGS) -o $(PROGRAM).vax $(LIB)

xtank.hp9k8 hp9k8: $(OBJS) $(HP9K8OBJS)
	$(CC) $(OBJS) $(HP9K8KOBJS) $(CFLAGS) -o $(PROGRAM).hp9k8 $(LIB)

init.o:
	$(CC) $(CCNOPT) -c init.c

grpobj:
	-chgrp xtank *.o 2>/dev/null
	-chmod 664 *.o 2>/dev/null

prof:
	$(CC) -c -O -pg $(SRCS)
	$(CC) $(OBJS) -O -pg -o $(PROGRAM).prof $(LIB)
	
lint:
	lint -uabhx $(DFLAGS) $(SRCS)

clean:
	rm -f $(OBJS) vaxsj.o tags TAGS

print:
	enscript -2rG Makefile $(INCL) $(SRCS)

TAGS: $(INCL) $(SRCS) $(XINCL) $(XSRCS) x11.c
	etags -t $(INCL) $(XINCL) $(SRCS) $(XSRCS) x11.c

# Include file dependency fixer-uppers

common.h:
	touch common.h
xtank.h:
	touch xtank.h
graphics.h:
	touch graphics.h
gr.h:
	touch gr.h
menu.h:
	touch menu.h

# header dependencies

3d.o: malloc.h terminal.h vehicle.h types.h message.h limits.h config.h
3d.o: structs.h xtanklib.h map.h screen.h common.h vehicle_parts.h object.h

Dio.o: xtanklib.h map.h screen.h config.h common.h message.h types.h
Dio.o: limits.h

Diophantine.o: xtanklib.h map.h screen.h config.h common.h message.h types.h
Diophantine.o: limits.h

Flipper.o: xtanklib.h map.h screen.h config.h common.h message.h types.h
Flipper.o: limits.h 

XMultiSync.o: malloc.h Xlibint.h Xlibos.h 

actions.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
actions.o: message.h types.h limits.h structs.h vehicle_parts.h object.h disc.h
actions.o: graphics.h x11.h gr.h vstructs.h bullet.h vehicle.h terminal.h
actions.o: cosell.h

animate.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
animate.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
animate.o: terminal.h vehicle.h

artful.o: xtanklib.h map.h screen.h config.h common.h message.h types.h
artful.o: limits.h

box.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
box.o: types.h limits.h structs.h vehicle_parts.h object.h vstructs.h box.h
box.o: sysdep.h vehicle.h

buddy.o: malloc.h xtanklib.h map.h screen.h config.h common.h message.h types.h
buddy.o: limits.h buddy.h

collision.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
collision.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
collision.o: vstructs.h bullet.h vehicle.h

console.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
console.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
console.o: graphics.h x11.h gr.h vstructs.h vehicle.h

cosell.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
cosell.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
cosell.o: cosell.h bullet.h vehicle.h

display.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
display.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
display.o: graphics.h x11.h gr.h vstructs.h bullet.h vehicle.h terminal.h

drone.o: malloc.h xtanklib.h map.h screen.h config.h common.h message.h types.h
drone.o: limits.h sysdep.h

eliza.o: malloc.h xtanklib.h map.h screen.h config.h common.h message.h types.h
eliza.o: limits.h

escher.o: graphics.h common.h config.h object.h types.h limits.h x11.h

file.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
file.o: types.h limits.h structs.h vehicle_parts.h object.h
file.o: mazeconv.c

game.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
game.o: types.h limits.h structs.h vehicle_parts.h object.h graphics.h x11.h
game.o: gr.h vehicle.h cosell.h

gr.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
gr.o: types.h limits.h structs.h vehicle_parts.h object.h graphics.h x11.h gr.h
gr.o: terminal.h vehicle.h

graphics.o: malloc.h config.h x11.c graphics.h common.h object.h types.h
graphics.o: limits.h x11.h

highlib.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
highlib.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
highlib.o: vehicle.h

hit.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
hit.o: types.h limits.h structs.h vehicle_parts.h object.h loc.h disc.h
hit.o: vstructs.h sysdep.h bullet.h vehicle.h terminal.h cosell.h

hud.o: xtanklib.h map.h screen.h config.h common.h message.h types.h limits.h

icounter.o: malloc.h icounter.h config.h common.h types.h

init.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
init.o: types.h limits.h structs.h vehicle_parts.h object.h graphics.h x11.h
init.o: vstructs.h bullet.h vehicle.h

input.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
input.o: types.h limits.h structs.h vehicle_parts.h object.h graphics.h x11.h
input.o: gr.h disc.h sysdep.h vehicle.h terminal.h

interface.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
interface.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
interface.o: graphics.h x11.h gr.h menu.h interface.h terminal.h vehicle.h

intersect.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
intersect.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
intersect.o: vehicle.h

kamikaze.o: xtanklib.h map.h screen.h config.h common.h message.h types.h
kamikaze.o: limits.h

lowlib.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
lowlib.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
lowlib.o: vstructs.h sysdep.h bullet.h vehicle.h cosell.h

main.o: malloc.h config.h xtank.h screen.h xtanklib.h map.h common.h message.h
main.o: types.h limits.h structs.h vehicle_parts.h object.h bullet.h vehicle.h

mapper.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
mapper.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
mapper.o: graphics.h x11.h gr.h vehicle.h terminal.h

maze.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
maze.o: types.h limits.h structs.h vehicle_parts.h object.h

mdesign.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
mdesign.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
mdesign.o: mdesign.h graphics.h x11.h gr.h terminal.h vehicle.h

menu.o: malloc.h common.h graphics.h config.h object.h types.h limits.h x11.h
menu.o: menu.h

message.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
message.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
message.o: graphics.h x11.h gr.h menu.h vehicle.h terminal.h

objects.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
objects.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
objects.o: graphics.h x11.h gr.h Objects/lightc.obj Objects/hexo.obj
objects.o: Objects/spider.obj Objects/psycho.obj Objects/tornado.obj
objects.o: Objects/marauder.obj Objects/tiger.obj Objects/rhino.obj
objects.o: Objects/medusa.obj Objects/malice.obj Objects/trike.obj
objects.o: Objects/turret_sm.obj Objects/shock.obj Objects/gleam.obj
objects.o: Objects/tink.obj Objects/soft.obj Objects/circle.obj
objects.o: Objects/fiery.obj Objects/double.obj Objects/exhaust.obj
objects.o: Objects/all_bullets.obj Objects/anim_landmarks.obj
objects.o: Objects/map_landmarks.obj Objects/design_landmarks.obj
objects.o: Objects/xtank.obj Objects/team.obj Objects/terp.obj
objects.o: Objects/electric.obj

program.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
program.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
program.o: thread.h icounter.h vehicle.h

radar.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
radar.o: types.h limits.h structs.h vehicle_parts.h object.h graphics.h x11.h
radar.o: gr.h vehicle.h

repair.o: xtank.h screen.h config.h xtanklib.h map.h common.h message.h types.h
repair.o: limits.h structs.h vehicle_parts.h object.h vstructs.h sysdep.h
repair.o: vehicle.h

scroll.o: xtank.h screen.h config.h xtanklib.h map.h common.h message.h types.h
scroll.o: limits.h structs.h vehicle_parts.h object.h graphics.h x11.h gr.h

setup.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
setup.o: types.h limits.h structs.h vehicle_parts.h object.h graphics.h x11.h
setup.o: gr.h vstructs.h terminal.h vehicle.h cosell.h

shooter.o: malloc.h xtanklib.h map.h screen.h config.h common.h message.h
shooter.o: types.h limits.h

spot.o: xtanklib.h map.h screen.h config.h common.h message.h types.h limits.h

status.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
status.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
status.o: graphics.h x11.h gr.h vehicle.h

thread.o: malloc.h thread.h config.h

unix.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h message.h
unix.o: types.h limits.h structs.h vehicle_parts.h object.h

update.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
update.o: message.h types.h limits.h structs.h vehicle_parts.h object.h disc.h
update.o: loc.h vstructs.h sysdep.h bullet.h vehicle.h terminal.h

util.o: malloc.h config.h xtank.h screen.h xtanklib.h map.h common.h message.h
util.o: types.h limits.h structs.h vehicle_parts.h object.h graphics.h x11.h
util.o: gr.h terminal.h vehicle.h

vdesign.o: malloc.h xtank.h screen.h config.h xtanklib.h map.h common.h
vdesign.o: message.h types.h limits.h structs.h vehicle_parts.h object.h
vdesign.o: graphics.h x11.h gr.h vstructs.h vdesign.h menu.h terminal.h
vdesign.o: vehicle.h

warrior.o: malloc.h xtanklib.h map.h screen.h config.h common.h message.h
warrior.o: types.h limits.h sysdep.h

x11.o: malloc.h graphics.h common.h config.h object.h types.h limits.h x11.h
