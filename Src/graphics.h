/*
** Graphics Toolkit
**
** Copyright 1988 by Terry Donahue
**
** graphics.h
*/

#include "common.h"
#include "config.h"
#include "object.h"

typedef struct
{
    int   win;
    Byte  type;
    int   x;
    int   y;
    char  key;
} Event;

#define EVENT_RBUTTON   0
#define EVENT_LBUTTON   1
#define EVENT_MBUTTON   2
#define EVENT_RBUTTONUP 3
#define EVENT_LBUTTONUP 4
#define EVENT_MBUTTONUP 5
#define EVENT_KEY       6
#define EVENT_MOVED     7

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
