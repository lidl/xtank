/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** program.h
*/

/* Time (in microseconds) a program is allowed to run */
#define PROGRAM_TIME 10000

/* Time (in microseconds) a program is allotted per frame */
#define PROGRAM_ALLOT 5000

/* Size of buffer put after each MPTHD */
#define THREAD_BUFSIZE 20000

/* Program ability flags */
#define PLAYS_COMBAT		(1<<0)
#define PLAYS_WAR		(1<<1)
#define PLAYS_ULTIMATE		(1<<2)
#define PLAYS_CAPTURE		(1<<3)
#define PLAYS_RACE		(1<<4)
#define DOES_SHOOT		(1<<5)
#define DOES_EXPLORE		(1<<6)
#define DOES_DODGE		(1<<7)
#define DOES_REPLENISH		(1<<8)
#define USES_TEAMS		(1<<9)
#define USES_MINES		(1<<10)
#define USES_SLICKS		(1<<11)
#define USES_SIDE_MOUNTS	(1<<12)
#define USES_MESSAGES    	(1<<13)

#define MAX_PROG_ABILITIES      14

