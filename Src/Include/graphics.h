/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
