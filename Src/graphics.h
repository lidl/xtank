/*
** Graphics Toolkit
**
** Copyright 1988 by Terry Donahue
**
** graphics.h
*/

#include "common.h"
#include "config.h"

typedef struct {
  int win;
  Byte type;
  int x;
  int y;
  char key;
} Event;

#ifndef _PICTURE_
#define _PICTURE_
typedef struct {
  int width;
  int height;
  int offset_x;
  int offset_y;
  int pixmap;
} Picture;
#endif _PICTURE_

#define EVENT_RBUTTON   0
#define EVENT_LBUTTON   1
#define EVENT_MBUTTON   2
#define EVENT_RBUTTONUP 3
#define EVENT_LBUTTONUP 4
#define EVENT_MBUTTONUP 5
#define EVENT_KEY       6
#define EVENT_MOVED     7

#define S_FONT	  0
#define M_FONT	  1
#define L_FONT	  2
#define XL_FONT	  3
#define MAX_FONTS 4

#define BLACK      0
#define WHITE      1
#define RED        2
#define ORANGE     3
#define YELLOW     4
#define GREEN      5
#define BLUE       6
#define VIOLET     7
#define MAX_COLORS 8

#define CROSS_CURSOR 0
#define PLUS_CURSOR  1
#define UL_CURSOR    2
#define LR_CURSOR    3
#define MAX_CURSORS  4

#define MAX_PIXMAPS	350
#define MAX_WINDOWS	12

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
