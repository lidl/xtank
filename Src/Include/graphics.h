/*
** Graphics Toolkit
**
** Copyright 1988 by Terry Donahue
**
** graphics.h
*/

/*
$Author: lidl $
$Id: graphics.h,v 2.5 1991/09/19 05:30:16 lidl Exp $

$Log: graphics.h,v $
 * Revision 2.5  1991/09/19  05:30:16  lidl
 * added KEYPAD_DETECT ifdef
 *
 * Revision 2.4  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.3  1991/02/10  13:50:39  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:53  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:34  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:31  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:26  aahz
 * Initial revision
 * 
*/

#include "common.h"
#include "object.h"


typedef struct {
    int   x, y;
    int   len;
    char *str;
} Word;

typedef enum {
    EVENT_RBUTTON, EVENT_LBUTTON,    EVENT_MBUTTON,
    EVENT_RBUTTONUP, EVENT_LBUTTONUP, EVENT_MBUTTONUP,
    EVENT_KEY,
    EVENT_MOVED,
} EventType;

typedef struct {
    int   win;
    EventType  type;
    int   x, y;
    char  key;
#ifdef KEYPAD_DETECT
    int   keypad;
#endif
} Event;


#define T_FONT    0	/* Tiny/Toddler - your pick */
#define S_FONT	  1
#define M_FONT	  2
#define L_FONT	  3
#define XL_FONT	  4
#define MAX_FONTS 5

#define BLACK      0
#define WHITE      1
#define RED        2
#define ORANGE     3
#define YELLOW     4
#define GREEN      5
#define BLUE       6
#define VIOLET     7
#define GREY       8	/* grey71 */
#define CUR_COLOR  9
#define MAX_COLORS 10

#define CROSS_CURSOR 0
#define PLUS_CURSOR  1
#define UL_CURSOR    2
#define LR_CURSOR    3
#define MAX_CURSORS  4

/* #define MAX_PIXMAPS	350 */
#define MAX_PIXMAPS	450		/* GHS */
#define MAX_WINDOWS	 16

#define LEFT_BORDER  5
#define TOP_BORDER   5

#ifdef X10
#include "x10.h"
#endif

#ifdef X11
#include "x11.h"
#endif

#ifdef AMIGA
#include "amigagfx.h"
#endif

Video *make_video();
