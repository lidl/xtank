/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** config.h
*/

/*
** The following defines indicate the operating system, graphics system,
** screen size of your machine, and whether you have XMultiSync or not.
*/

/* Currently, the program compiles and runs when UNIX, X11 and
** S1024x864 are set.  It will not work if you set AMIGA, X10, or
** S640x400.  Things will work whether MULTI_SYNC is set or not.
** If you cannot get XMultiSync.c to compile, then you should
** remove the #define MULTI_SYNC at the end of this file.
*/

/* UNIX or AMIGA */
#define UNIX

/* X10 or X11 or AMIGA */
#define X11

/* S1024x864 or S640x400 */
#define S1024x864

/* MULTI_SYNC or nothing */
#define MULTI_SYNC
