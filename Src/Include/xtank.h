/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** xtank.h
*/

/*
$Author: lidl $
$Id: xtank.h,v 2.10 1992/08/31 01:50:45 lidl Exp $

$Log: xtank.h,v $
 * Revision 2.10  1992/08/31  01:50:45  lidl
 * changed to use tanktypes.h, instead of types.h
 *
 * Revision 2.9  1992/08/18  05:42:39  lidl
 * added tac nuke patches
 *
 * Revision 2.8  1992/03/31  21:49:23  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.7  1992/01/29  08:39:11  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.6  1991/12/03  20:15:57  lidl
 * botched try for prototype support
 *
 * Revision 2.5  1991/11/23  06:31:27  lidl
 * now includes the function prototypes file
 *
 * Revision 2.4  1991/10/27  22:35:15  aahz
 * updated the number of weapons.
 *
 * Revision 2.3  1991/02/10  13:52:15  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:33  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:45  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:59  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:24  aahz
 * Initial revision
 * 
*/

#ifndef _XTANK_H_
#define _XTANK_H_

#include <stdio.h>
#if defined(SYSV) || defined(SVR4)
#include <string.h>
#else
#include <strings.h>
#endif

#include <math.h>
#include "screen.h"
#include "xtanklib.h"		/* many important things here */
/* #include "proto.h"		/* function prototypes for ANSI compilers */

#ifdef AMIGA
#include "amiga.h"
#endif

#ifdef TEST_TURRETS
#define TURRET_LENGTH 25
#endif /* TEST_TURRETS */

/* Number of frames animation lasts after end of game */
#define QUIT_DELAY 17
/* Number of frames you get to keep watching after your vehicle is killed */
#define DEATH_DELAY 30

/* Maze geometry information */
#define MAZE_HEIGHT	26
#define MAZE_WIDTH	26
#define MAZE_TOP	2
#define MAZE_BOTTOM	27
#define MAZE_LEFT	2
#define MAZE_RIGHT	27

/* Additional flags used in maze */
#define BOX_CHANGED	(1<<5)
#define VEHICLE_0	(1<<8)
#define ANY_VEHICLE	0x0fffff00

/* General max values */
#define MAX_WEAPON_STATS 20
#define MAX_GAME_SPEED   30

/* Description max values */
#define MAX_VDESCS	100
#define MAX_MDESCS	100
#define MAX_PDESCS	30
#define MAX_SDESCS	30

/* Flags for display routines */
#define OFF	        0
#define ON	        1
#define REDISPLAY       2

/* Return values for animation routine */
#define GAME_FAILED	       (-1)
#define GAME_RUNNING		0
#define GAME_OVER		1
#define GAME_QUIT		2
#define GAME_RESET		3
#define SWAPPED         4

/* Return values for description loading */
#define DESC_LOADED     0
#define DESC_SAVED	1
#define DESC_NOT_FOUND  2
#define DESC_BAD_FORMAT 3
#define DESC_NO_ROOM    4

/*
 * eithier this or use different symbols for do_special, 
 * ie not SP_xxx
 * this seemed simple enough
 */

#define SP_update	(0+MAX_SPEC_STATS)
#define SP_activate	(1+MAX_SPEC_STATS)
#define SP_deactivate	(2+MAX_SPEC_STATS)
#define SP_toggle	(3+MAX_SPEC_STATS)
#define SP_draw		(4+MAX_SPEC_STATS)
#define SP_erase	(5+MAX_SPEC_STATS)
#define SP_redisplay	(6+MAX_SPEC_STATS)
#define SP_break	(7+MAX_SPEC_STATS)
#define SP_repair	(8+MAX_SPEC_STATS)

/* Vehicle, weapon, and program status masks */
#define VS_functioning		(1<<0)
#define VS_is_alive		(1<<1)
#define VS_was_alive		(1<<2)
#define VS_disc_spin		(1<<3)
#define VS_rel_turret		(1<<4)
#define VS_sliding		(1<<5)
#define VS_permanently_dead	(1<<6)

#define WS_on			(1<<0)
#define WS_func			(1<<1)
#define WS_no_ammo		(1<<2)

#define TS_3d			(1<<0)
#define TS_wide 		(1<<1)
#define TS_long 		(1<<2)
#define TS_extend 		(1<<3)
#define TS_clip 		(1<<4)

#define PROG_on                 (1<<0)

/* Types of explosions (move to xtanklib.h?) */
#define EXP_TANK      0
#define EXP_GLEAM     1
#define EXP_DAM0      2
#define EXP_DAM1      3
#define EXP_DAM2      4
#define EXP_DAM3      5
#define EXP_DAM4      6
#define EXP_EXHAUST   7
#define EXP_ELECTRIC  8
#ifdef TAC_NUKES
#define EXP_NUKE      9
#endif

/* Types of descriptions */
#define VDESC 0
#define MDESC 1
#define SDESC 2

/* 3d elevation information */
#define WALLTOP_Z        (BOX_WIDTH/4)
#define WALLBOTTOM_Z     (-BOX_WIDTH/4)
#define TURRET_MOUNT_Z   (BOX_WIDTH/8)
#define SIDE_MOUNT_Z     (-BOX_WIDTH/8)


#endif /* _XTANK_H_ */
