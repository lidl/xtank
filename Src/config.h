/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** config.h
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_


/* Operating system:  UNIX or AMIGA
** Currently, xtank will not work if AMIGA is set here.
*/
#define UNIX

/* Sun operating system:  SUNOS4_0 or SUNOS3_0 or leave blank
** If you are compiling for a sun, you should enter a
** #define SUNOS4_0 or #define SUNOS3_0 here.
*/
#define SUNOS4_0

/* Graphics system:   X10 or X11 or AMIGA
** The program will not work if X10 or AMIGA is set here.
*/
#define X11

/* Thread package:    THREAD_MP or THREAD_SUNLWP or nothing
**
** The standard threads package is THREAD_MP, which runs on the
** machines listed in thread.h.  To use it, #define THREAD_MP below.
**
** There is an alternate threads package available on Suns which uses
** the LWP library.  To use it, #define THREAD_SUNLWP below, and
** do a "make sunlwp" to use the lwp library.
**
** If you cannot get threads working using any of these methods, you
** will not be able to have computer controlled vehicles.  Remove
** the #define below.
*/
#define THREAD_SUNLWP

/* X11R2 fonts:
** If your machine is using X11R2 fontnames, instead of X11R3 fontnames,
** #define X11R2FONTS here.
*/

/* Screen size:       S1024x864 or S640x400
** The program will not work if S640x400 is set here.
** If your screen is smaller than 1024x864, then some things will not
** be visible (only the help window will be invisible on 1024x768 screens).
*/
#define S1024x864

/*
** Put the path to the main xtank directory in quotes in the #define below.
** This is used as backup in case the environment variable XTANK_DIR is not set
** at runtime.  The environment variable XTANK_DIR is set in the shell script
** called xtank in the main directory.  You should change it there also.
*/
#define XTANK_DIR "/homes/elves/stripes/xtank"

/* If you cannot get XMultiSync.c to compile, remove this #define MULTI_SYNC.
** This will make multi-player games noticeably slower, though.
*/
#define MULTI_SYNC

/* If you do not want to restrict multi-player games to have all machines
** on the same subnet of your network, remove this #define CHECKNET.
*/
/* #define CHECKNET */

/* If you are not able to get compile_module() to work in unix.c,
** remove this #define DYNALOAD.  You will then not be able to add new
** robot programs without recompiling.
*/
#define DYNALOAD

/* If you want to see some debugging information in main.c and unix.c,
** add a #define DEBUG here.
*/
/* If you need to use the auxiliary font add a #define NEED_AUX_FONT
*/
/* #define NEED_AUX_FONT */

#endif ndef _CONFIG_H_
