/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** x11.h
*/

#include <X11/Xlib.h>

#define WIN_exposed	(1<<0)
#define WIN_mapped      (1<<1)

#define DRAW_XOR	0
#define DRAW_COPY	1
#define MAX_DRAW_FUNCS	2

/*********************
** Window functions **
*********************/

#define win_exposed(w) \
   (vid->win[w].flags & WIN_exposed)

#define win_mapped(w) \
   (vid->win[w].flags & WIN_mapped)

#define expose_win(w,status) \
  (status ? (vid->win[w].flags |= WIN_exposed) : \
            (vid->win[w].flags &= ~WIN_exposed))

/**********************
** Drawing functions **
**********************/

#define draw_point(w,x,y,func,color) \
  (XDrawPoint(vid->dpy,vid->win[w].id,vid->graph_gc[func][color],x,y))

#define draw_points(w,points,num_points,x,y,func,color) \
  (XDrawPoints(vid->dpy,vid->win[w].id,vid->graph_gc[func][color], \
	      points,num_points,CoordModeOrigin))

#define draw_line(w,x1,y1,x2,y2,func,color) \
  (XDrawLine(vid->dpy,vid->win[w].id,vid->graph_gc[func][color],x1,y1,x2,y2))

#define draw_lines(w,lines,num_lines,func,color) \
  (XDrawSegments(vid->dpy,vid->win[w].id,vid->graph_gc[func][color], \
	     lines,num_lines,CoordModeOrigin))

#define draw_vert(w,x,y,size,func,color) \
  (draw_line(w,x,y,x,y+size-1,func,color))

#define draw_hor(w,x,y,size,func,color) \
  (draw_line(w,x,y,x+size-1,y,func,color))

#define draw_rect(w,x,y,width,height,func,color) \
  (XDrawRectangle(vid->dpy,vid->win[w].id,vid->graph_gc[func][color], \
		  x,y,width,height))

#define draw_filled_rect(w,x,y,width,height,func,color) \
  (XFillRectangle(vid->dpy,vid->win[w].id,vid->graph_gc[func][color], \
		  x,y,width,height))

#define draw_square(w,x,y,size,func,color) \
  (draw_rect(w,x,y,size,size,func,color))

#define draw_filled_square(w,x,y,size,func,color) \
  (draw_filled_rect(w,x,y,size,size,func,color))

#define draw_picture(w,x,y,pic,func,color) \
  do { \
    int tmp_x = x - pic->offset_x; \
    int tmp_y = y - pic->offset_y; \
      if(x > -pic->offset_x && tmp_x < vid->win[w].width && \
	 y > -pic->offset_y && tmp_y < vid->win[w].height) { \
	if(vid->planes == 1) \
	  XCopyArea(vid->dpy,vid->pixid[pic->pixmap],vid->win[w].id, \
	 	    vid->graph_gc[func][color],0,0,pic->width,pic->height, \
		    tmp_x,tmp_y); \
        else \
	  XCopyPlane(vid->dpy,vid->pixid[pic->pixmap],vid->win[w].id, \
		     vid->graph_gc[func][color],0,0,pic->width,pic->height, \
		     tmp_x,tmp_y,0x1); \
      } \
  } while (0)

typedef struct {
  Window id;
  int width,height;
  int flags;
} Win;

typedef struct {
  char display_name[50];
  Display *dpy;
  int planes;
  unsigned int input_mask;
  Window parent_id;
  int num_windows;
  Win win[MAX_WINDOWS];
  GC graph_gc[MAX_DRAW_FUNCS][MAX_COLORS];
  GC text_gc[MAX_FONTS][MAX_DRAW_FUNCS][MAX_COLORS];
  XFontStruct *fs[MAX_FONTS];
  int num_pixids;
  Pixmap pixid[MAX_PIXMAPS];
  unsigned long fg,bg,color[MAX_COLORS];
  Cursor cursor[MAX_CURSORS];
} Video;

/*
typedef XSegment Line;
typedef XPoint Point;
*/

extern Video *vid;
