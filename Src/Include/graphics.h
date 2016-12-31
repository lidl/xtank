/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

/*
** Comment: Graphics Toolkit
*/

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "common.h"
#include "vehicleparts.h"
#include "object.h"

  typedef struct {
	  int x, y;
	  int len;
	  char *str;
  }
Word;

  typedef enum {
	  EVENT_RBUTTON, EVENT_LBUTTON, EVENT_MBUTTON,
	  EVENT_RBUTTONUP, EVENT_LBUTTONUP, EVENT_MBUTTONUP,
	  EVENT_KEY,
	  EVENT_MOVED
  } EventType;

  typedef struct {
	  int win;
	  EventType type;
	  int x, y;
	  char key;
#ifdef KEYPAD_DETECT
	  int keypad;
#endif
  }
Event;


#define T_FONT    0				/* Tiny/Toddler - your pick */
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
#define GREY       8			/* grey71 */
#define CUR_COLOR  9
#define DASHED     10
#define MAX_COLORS 11

#define CROSS_CURSOR 0
#define PLUS_CURSOR  1
#define UL_CURSOR    2
#define LR_CURSOR    3
#define MAX_CURSORS  4

/* #define MAX_PIXMAPS	350 */
#define MAX_PIXMAPS	500		/* GHS */ /*HAK*/
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

#endif /* _GRAPHICS_H_ */
