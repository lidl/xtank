/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** x11.c
*/

#include <stdio.h>
#include <strings.h>
#include "graphics.h"
#include <X11/Xutil.h>
#include <ctype.h>

extern char *malloc();

extern char *program_name;
#define icon_width 64
#define icon_height 14
static char icon_bits[] = {
   0x07, 0x98, 0xff, 0xc7, 0x03, 0x0f, 0xe7, 0x30, 0x0f, 0xbc, 0xff, 0xef,
   0x03, 0x1f, 0xef, 0x79, 0x1b, 0x2e, 0x01, 0xe4, 0x06, 0x23, 0x6b, 0x5d,
   0x36, 0x17, 0xde, 0x73, 0x05, 0x2b, 0x6b, 0x6f, 0xec, 0x0b, 0x58, 0xb0,
   0x0b, 0x5b, 0x6b, 0x37, 0xd8, 0x05, 0x58, 0xb8, 0x0a, 0x5b, 0x6b, 0x1b,
   0xb0, 0x02, 0x58, 0xd8, 0x17, 0xab, 0x6b, 0x0d, 0x70, 0x03, 0x58, 0xdc,
   0x17, 0xab, 0x6b, 0x06, 0xb8, 0x06, 0x58, 0x0c, 0x20, 0x4b, 0x6b, 0x0d,
   0xdc, 0x0d, 0x58, 0xee, 0x2f, 0x4b, 0x6b, 0x1b, 0x2e, 0x1b, 0x58, 0x16,
   0x50, 0x8b, 0x6a, 0x35, 0x17, 0x36, 0x58, 0x17, 0x50, 0x8b, 0x6a, 0x69,
   0x0b, 0x2c, 0x58, 0x0b, 0xa0, 0x0b, 0x69, 0x51, 0x06, 0x38, 0x70, 0x0e,
   0xe0, 0x0e, 0xcf, 0x61};

static char *colorname[MAX_COLORS] = {
  "Black", "White", "Red", "Orange", "Yellow", "Green", "Blue", "Violet" };
static char *fontname[MAX_FONTS] = {
  "6x10", "*-courier-bold-r-*-14-*", "*-courier-bold-r-*-18-*",
  "*-courier-bold-r-*-24-*" };
static int fwidth[MAX_FONTS],fheight[MAX_FONTS],fascent[MAX_FONTS];

Video *vid;

/******************
** Error Handler **
******************/

char video_error_str[256];

#define video_error(identifier,arg) \
  sprintf(video_error_str,identifier,arg)

/*********************
** Window functions **
*********************/

/*
** Initializes everything dealing with graphics.
*/
open_graphics()
{
}

/*
** Frees all allocated storage for graphics.
*/
close_graphics()
{
}

/*
** Sets the current video.
*/
set_video(video)
     Video *video;
{
  vid = video;
}

#define punt() \
  { close_video(video); \
    return (Video *) NULL; }

/*
** Makes a video structure, initializes it.
** Returns a pointer to the video or NULL if video could not be initialized.
*/
Video *make_video(name)
     char *name;
{
  Video *video;

  video = (Video *) malloc(sizeof(Video));
  video->dpy = XOpenDisplay(name);
  (void) strcpy(video->display_name,name);
  if(video->dpy == NULL) {
    video_error("Could not open display named %s",name);
    free((char *) video);
    return (Video *) NULL;
  }

  set_video(video);

  /* Make the gcs, cursors, and parent window for the display */
  if(make_gcs()) punt();
  if(make_cursors()) punt();
  if(make_parent()) punt();

  video->num_windows = 0;
  video->num_pixids = 0;
  return video;
}

/*
** Makes a full screen parent window for the application.
*/
make_parent()
{
  Window rw;
  XSizeHints size;
  Pixmap icon;

  size.flags = USPosition | USSize;
  size.x = 0;
  size.y = 0;
  size.width = size.min_width = size.max_width =
    DisplayWidth(vid->dpy,DefaultScreen(vid->dpy));
  size.height = size.min_height = size.max_height =
    DisplayHeight(vid->dpy,DefaultScreen(vid->dpy));
  rw = RootWindow(vid->dpy,DefaultScreen(vid->dpy));

  vid->parent_id = XCreateSimpleWindow(vid->dpy,rw,
	       size.x,size.y,size.width,size.height,0,vid->fg,vid->bg);
  if(vid->parent_id == NULL) {
    video_error("Could not open parent window %s","");
    return 1;
  }

  icon = XCreateBitmapFromData(vid->dpy,rw,icon_bits,icon_width,icon_height);
  XSetStandardProperties(vid->dpy,vid->parent_id,program_name,program_name,
			 icon,NULL,0,&size);

  return 0;
}

/*
** Destroys the specified video.
*/
close_video(video)
     Video *video;
{
  XCloseDisplay(video->dpy);
  free((char *) video);
}

/*
** Makes a window with the specified characteristics.
*/
make_window(w,x,y,width,height,border)
     int w,x,y,width,height,border;
{
  Window id;
  
  id = XCreateSimpleWindow(vid->dpy,vid->parent_id,x,y,width,height,
			   border,vid->fg,vid->bg);
  if(id == NULL) {
    video_error("Could not open window #%d",w);
    return 1;
  }

  vid->input_mask = ButtonPressMask | KeyPressMask | ExposureMask;
  XSelectInput(vid->dpy,id,vid->input_mask);

  /* Make the internal window structure */
  vid->win[w].id = id;
  vid->win[w].width = width;
  vid->win[w].height = height;
  vid->win[w].flags = 0;

  /* Make sure the number of windows is high enough */
  vid->num_windows = max(vid->num_windows,w+1);

  return 0;
}

/*
** Destroys the window.
*/
close_window(w)
     int w;
{
  XDestroyWindow(vid->dpy,vid->win[w].id);
}

/*
** Displays the window on the screen.
*/
map_window(w)
     int w;
{
  /* Map the parent window just before mapping the first child */
  if(w == 0)
    XMapWindow(vid->dpy,vid->parent_id);
  XMapWindow(vid->dpy,vid->win[w].id);
  vid->win[w].flags |= WIN_mapped;
}

/*
** Removes the window from the screen.
*/
unmap_window(w)
     int w;
{
  XUnmapWindow(vid->dpy,vid->win[w].id);
  vid->win[w].flags &= ~WIN_mapped;
}

/*
** Clears the window.
*/
clear_window(w)
     int w;
{
  XClearWindow(vid->dpy,vid->win[w].id);
}

/***********************
** Graphics functions **
***********************/

/*
** Use this routine to draw text in an arbitrary position in a window
** The x coordinate specifies the center of the string to be drawn.
** The y coordinate is at the top of the string.
*/
draw_text(w,x,y,str,font,func,color)
     int w;
     int x,y;
     char *str;
     int font,func,color;
{
  int len;

  len = strlen(str);
  XDrawString(vid->dpy,vid->win[w].id,vid->text_gc[font][func][color],
	      x - fwidth[font] * len / 2,
	      y + fascent[font],
	      str,len);
}

/*
** Use this routine to write text in a text type window
** The x and y values in the word structure are interpreted as
** a row and column in the window.
*/
draw_text_rc(w,x,y,str,font,color)
     int w,x,y;
     char *str;
     int font,color;
{
  XDrawImageString(vid->dpy,vid->win[w].id,
		   vid->text_gc[font][DRAW_COPY][color],
		   LEFT_BORDER + fwidth[font] * x,
		   TOP_BORDER + fheight[font] * y + fascent[font],
		   str,strlen(str));
}

/*
** Clears a rectangle of text rows and columns in a specified window.
*/
clear_text_rc(w,x,y,width,height,font)
     int w,x,y,width,height,font;
{
  draw_filled_rect(w,
		   LEFT_BORDER + x * fwidth[font],
		   TOP_BORDER + y * fheight[font],
		   width * fwidth[font],
		   height * fheight[font],
		   DRAW_COPY,BLACK);
}

/******************
** I/O functions **
******************/

/*
** Performs all graphics operations that have been queued up.
*/
flush_output()
{
  XFlush(vid->dpy);
}

/*
** Performs all graphics operations and waits until all input is received.
*/
sync_output(discard)
     Boolean discard;
{
  XSync(vid->dpy,discard);
}

#ifdef MULTI_SYNC
/*
** Synchronizes all the given video displays.
** Currently the number of videos is limited to 10.
*/
multi_sync(video,num_videos,discard)
     Video *video[];
     int num_videos;
     Boolean discard;
{
  Display *dpys[10];
  int i;

  for(i = 0 ; i < num_videos ; i++)
    dpys[i] = video[i]->dpy;

  XMultiSync(dpys,num_videos,discard);
}
#endif

/*
** Gets up to *num_events input events, handling the expose events
** by setting the WIN_expose flag.  Events are stored in the event
** array, with num_events holding the number of events placed there.
*/
get_events(num_events,event)
     int *num_events;
     Event event[];
{
  XEvent xevent;
  XAnyEvent *any_xevent;
  XKeyEvent *key_xevent;
  XButtonEvent *button_xevent;
  XMotionEvent *motion_xevent;
  Event *e;
  char buf[2];
  int max_events;
  int w,tf;

  max_events = *num_events;
  *num_events = 0;
  while(*num_events < max_events && XPending(vid->dpy)) {
    XNextEvent(vid->dpy,&xevent);
    any_xevent = &xevent.xany;
    for(w = 0 ; w < vid->num_windows ; w++)
      if(any_xevent->window == vid->win[w].id) {
	switch((int) xevent.type) {
	  case Expose:
	    /* Set the exposed bit in the window flags if win is exposed */
	    vid->win[w].flags |= WIN_exposed;
	    break;
	  case KeyPress:
	    /* Figure out which key was pressed (if any) */
	    key_xevent = &xevent.xkey;
	    if(XLookupString(key_xevent,buf,1,NULL,NULL) == 0) break;
	    if(!isprint(buf[0]) && buf[0] != '\r' && buf[0] != 127) break;

	    /* Add a key event to the array */
	    e = &event[(*num_events)++];
	    e->win = w;
	    e->type = EVENT_KEY;
	    e->key = buf[0];
	    e->x = key_xevent->x;
	    e->y = key_xevent->y;
	    break;
	  case ButtonPress:
	  case ButtonRelease:
	    /* Add a button event to the array */
	    e = &event[(*num_events)++];
	    e->win = w;

	    button_xevent = &xevent.xbutton;
	    tf = (xevent.type == ButtonPress);
	    switch(button_xevent->button) {
              case Button1: e->type = (tf?EVENT_LBUTTON:EVENT_LBUTTONUP);break;
              case Button2: e->type = (tf?EVENT_MBUTTON:EVENT_MBUTTONUP);break;
              case Button3: e->type = (tf?EVENT_RBUTTON:EVENT_RBUTTONUP);break;
	      }
	    e->x = button_xevent->x;
	    e->y = button_xevent->y;
	    break;
	  case MotionNotify:
	    /* Add a moved event to the array */
	    e = &event[(*num_events)++];
	    e->win = w;

	    motion_xevent = &xevent.xmotion;
	    e->type = EVENT_MOVED;
	    e->x = motion_xevent->x;
	    e->y = motion_xevent->y;
	    break;
	  }
      }
  }
}

/*
** Causes EVENT_MOVED to be enabled or disabled in get_events().  
*/
follow_mouse(w,status)
     int w;
     Boolean status;
{
  if(status == TRUE) vid->input_mask |= PointerMotionMask;
  else               vid->input_mask &= ~PointerMotionMask;

  XSelectInput(vid->dpy,vid->win[w].id,vid->input_mask);
}
	      
/*
** Causes EVENT_*BUTTONUP to be enabled or disabled in get_events().
*/
button_up(w,status)
     int w;
     Boolean status;
{
  if(status == TRUE) vid->input_mask |= ButtonReleaseMask;
  else               vid->input_mask &= ~ButtonReleaseMask;

  XSelectInput(vid->dpy,vid->win[w].id,vid->input_mask);
}

/****************************
** Cursors, fonts, pixmaps **
****************************/


/*
** Gets default resources, allocates colors, loads in fonts and makes the gcs.
*/
make_gcs()
{
  Window rw;
  XGCValues values;
  XCharStruct *cs;
  Colormap cmap;
  XColor scol,ecol;
  int scr,cells;
  unsigned long black,white;
  Visual *vis;
  char *fc, *bc, *rv;
  int mask;
  int i,j,k;
  static Boolean firsttime = TRUE;

  /* Get default info about screen */
  scr         = DefaultScreen(vid->dpy);
  cmap        = DefaultColormap(vid->dpy,scr);
  cells       = DisplayCells(vid->dpy,scr);
  vid->planes = DisplayPlanes(vid->dpy,scr);
  black       = BlackPixel(vid->dpy,scr);
  white       = WhitePixel(vid->dpy,scr);
  vis         = DefaultVisual(vid->dpy,scr);
  
  /* Get default info from resources */
  fc = XGetDefault(vid->dpy, program_name, "foreground");
  bc = XGetDefault(vid->dpy, program_name, "background");
  rv = XGetDefault(vid->dpy, program_name, "reverseVideo");

  /* Get foreground and background pixel values */
  if(rv != NULL &&
     (!strcmp(rv,"true") || !strcmp(rv,"on") || !strcmp(rv,"yes"))) {
    vid->fg = black;
    vid->bg = white;
  }
  else {
    vid->fg = white;
    vid->bg = black;
  }

  if(vid->planes > 1) {
    if(fc != NULL && XAllocNamedColor(vid->dpy,cmap,fc,&scol,&ecol))
      vid->fg = scol.pixel;

    if(bc != NULL && XAllocNamedColor(vid->dpy,cmap,bc,&scol,&ecol))
      vid->bg = scol.pixel;
  }

  /* Get array of pixel values */
  for(i = 0 ; i < MAX_COLORS ; i++)
    vid->color[i] = (i == BLACK) ? vid->bg : vid->fg;

  if(vid->planes > 1) {
    for(i = 2 ; i < MAX_COLORS ; i++)
      if(XAllocNamedColor(vid->dpy,cmap,colorname[i],&scol,&ecol))
	vid->color[i] = scol.pixel;
  }

  /* Load in all the fonts */
  for(i = 0 ; i < MAX_FONTS ; i++) {
    vid->fs[i] = XLoadQueryFont(vid->dpy,fontname[i]);
    if(!vid->fs[i]) vid->fs[i] = XLoadQueryFont(vid->dpy,"fixed");
    if(!vid->fs[i]) {
      video_error("Cannot open font named %s or fixed",fontname[i]);
      return 1;
    }
  }

  /* If first time, cache the widths, heights and ascents of the fonts */
  if(firsttime == TRUE) {
    firsttime = FALSE;
    for(i = 0 ; i < MAX_FONTS ; i++) {
      cs = &vid->fs[i]->max_bounds;
      fwidth[i] = cs->width;
      fheight[i] = cs->ascent + cs->descent - (i>0);
      fascent[i] = cs->ascent;
    }
  }

  rw = RootWindow(vid->dpy,scr);

  /*
  ** Turn off graphics exposures, to avoid getting an event for each
  ** XCopyArea with a pixmap.  X is totally brain damaged.
  */
  values.background = vid->bg;
  values.graphics_exposures = False;
  for(i = 0 ; i < MAX_COLORS ; i++) {
    for(j = 0 ; j < MAX_DRAW_FUNCS ; j++) {
       switch(j) {
	  case DRAW_XOR:
	    values.function = GXxor;
	    values.foreground = vid->color[i] ^ vid->bg;
	    break;
	  case DRAW_COPY:
	    values.function = GXcopy;
	    values.foreground = vid->color[i];
	    break;
	  }

       /* Make a gc for each font in this color,func */
       mask = GCForeground | GCBackground | GCFont |
	      GCFunction | GCGraphicsExposures;
       for(k = 0 ; k < MAX_FONTS ; k++) {
	  values.font = vid->fs[k]->fid;
	  vid->text_gc[k][j][i] = XCreateGC(vid->dpy,rw,mask,&values);
       }

       /* Make a gc for drawing in this color,func */
       mask = GCForeground | GCBackground | GCFunction | GCGraphicsExposures;
       vid->graph_gc[j][i] = XCreateGC(vid->dpy,rw,mask,&values);
    }
  }
  return 0;
}

#define cross_width 16
#define cross_height 16
#define cross_x_hot 7
#define cross_y_hot 7
static char cross_bits[] = {
   0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xc0, 0x01, 0x80, 0x00,
   0x10, 0x04, 0x3f, 0x7e, 0x10, 0x04, 0x80, 0x00, 0xc0, 0x01, 0x80, 0x00,
   0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00};
static char cross_mask[] = {
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xe0, 0x03, 0xd0, 0x05,
   0xbf, 0x7e, 0x7f, 0x7f, 0xbf, 0x7e, 0xd0, 0x05, 0xe0, 0x03, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0x00, 0x00};
#define plus_width 16
#define plus_height 16
#define plus_x_hot 8
#define plus_y_hot 7
static char plus_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 
    0xf0, 0x1f, 0xf0, 0x1f, 0xf0, 0x1f, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static char plus_mask[] = {
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xf8, 0x3f,
   0xf8, 0x3f, 0xf8, 0x3f, 0xf8, 0x3f, 0xf8, 0x3f, 0xc0, 0x07, 0xc0, 0x07,
   0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define ul_width 16
#define ul_height 16
#define ul_x_hot 8
#define ul_y_hot 7
static char ul_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x80, 0x3f, 0x80, 0x3f, 0x80, 0x3f, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 
    0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static char ul_mask[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x7f,
   0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x07, 0xc0, 0x07,
   0xc0, 0x07, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00};
#define lr_width 16
#define lr_height 16
#define lr_x_hot 8
#define lr_y_hot 7
static char lr_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 
    0xf8, 0x03, 0xf8, 0x03, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static char lr_mask[] = {
   0x00, 0x00, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xfc, 0x07,
   0xfc, 0x07, 0xfc, 0x07, 0xfc, 0x07, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
** Makes all cursors.
*/
make_cursors()
{
  int ret = 0;

  /* Create all the cursors */
  ret += make_cursor(CROSS_CURSOR,cross_width,cross_height,cross_x_hot,
		     cross_y_hot,cross_bits,cross_mask);
  ret += make_cursor(PLUS_CURSOR,plus_width,plus_height,plus_x_hot,
		     plus_y_hot,plus_bits,plus_mask);
  ret += make_cursor(UL_CURSOR,ul_width,ul_height,ul_x_hot,ul_y_hot,
		     ul_bits,ul_mask);
  ret += make_cursor(LR_CURSOR,lr_width,lr_height,lr_x_hot,lr_y_hot,
		     lr_bits,lr_mask);
  return ret;
}

/*
** Makes a cursor.
*/
make_cursor(c,width,height,xhot,yhot,bits,mask)
     int c,width,height,xhot,yhot;
     char bits[],mask[];
{
  XColor fcol,bcol;
  Colormap cmap;
  Pixmap bpix,mpix;

  /* Make the pixmap for the cursor and make the cursor */
  bpix = XCreateBitmapFromData(vid->dpy,
			       RootWindow(vid->dpy,DefaultScreen(vid->dpy)),
			       bits,width,height);
  mpix = XCreateBitmapFromData(vid->dpy,
			       RootWindow(vid->dpy,DefaultScreen(vid->dpy)),
			       mask,width,height);
  if(bpix == NULL || mpix == NULL) {
    video_error("Could not create cursor pixmap #%d",c);
    return 1;
  }

  /* Get foreground and background colors */
  cmap = DefaultColormap(vid->dpy,DefaultScreen(vid->dpy));
  fcol.pixel = vid->fg;
  XQueryColor(vid->dpy,cmap,&fcol);
  bcol.pixel = vid->bg;
  XQueryColor(vid->dpy,cmap,&bcol);

  vid->cursor[c] = XCreatePixmapCursor(vid->dpy,bpix,mpix,&fcol,&bcol,
				       xhot,yhot);
  XFreePixmap(vid->dpy,bpix);
  XFreePixmap(vid->dpy,mpix);
  return 0;
}

/*
** Sets the current cursor.
*/
set_cursor(c)
     int c;
{
  int i;

  if(c < 0 || c >= MAX_CURSORS) return;

  /* Define the cursor for all the windows */
  for(i = 0 ; i < vid->num_windows ; i++)
    XDefineCursor(vid->dpy,vid->win[i].id,vid->cursor[c]);
}

/*
** Returns height of the specified font.
*/
font_height(font)
     int font;
{
  return(fheight[font]);
}

/*
** Returns width of the string in the specified font.
*/
font_string_width(str,font)
     char *str;
     int font;
{
  return(fwidth[font] * strlen(str));
}

/*
** Makes the picture from the bitmap and the size given in the picture.
*/
make_picture(pic,bitmap)
     Picture *pic;
     unsigned char *bitmap;
{
  Pixmap temp;

  temp = XCreateBitmapFromData(vid->dpy,
			       RootWindow(vid->dpy,DefaultScreen(vid->dpy)),
			       bitmap,pic->width,pic->height);
  if(temp == NULL) {
    video_error("Could not store pixmap #%d",vid->num_pixids);
    return 1;
  }

  pic->pixmap = vid->num_pixids++;
  vid->pixid[pic->pixmap] = temp;
  return 0;
}

/*
** Frees the picture.
*/
free_picture(pic)
     Picture *pic;
{
  XFreePixmap(vid->dpy,vid->pixid[pic->pixmap]);
}

/*
** Puts a copy of pic rotated 90 degrees into rot_pic.
*/
Byte *rotate_pic_90(pic,rot_pic,bitmap)
     Picture *pic,*rot_pic;
     Byte *bitmap;
{
  Byte *rot_bitmap;
  int by_line,rot_by_line,num_bytes,rot_num_bytes;
  register Byte *s_ptr,*d_ptr,*s_ptr_begin,*d_ptr_begin;
  register Byte d_mask;
  register int s_bit_begin,d_bit;

  rot_pic->width = pic->height;
  rot_pic->height = pic->width;
  rot_pic->offset_x = pic->height - pic->offset_y - 1;
  rot_pic->offset_y = pic->offset_x; 
  
  by_line = (pic->width + 7) >> 3;
  num_bytes = by_line * pic->height;
  rot_by_line = (rot_pic->width + 7) >> 3;
  rot_num_bytes = rot_by_line * rot_pic->height;
  rot_bitmap = (Byte *) calloc((unsigned) (rot_num_bytes + 4),sizeof(Byte));

  /*
  ** Scan across each source scanline in the bitmap starting
  ** from the bottom right corner and working left and up.
  */
  s_ptr_begin = bitmap + num_bytes - 1;
  s_bit_begin = (pic->width - 1) & 0x7;
  d_ptr_begin = rot_bitmap + rot_num_bytes - rot_by_line;
  d_bit = 0;
  s_ptr = s_ptr_begin;
  do {
    d_ptr = d_ptr_begin;
    d_mask = 1 << d_bit;
    s_ptr_begin -= by_line;

    switch(s_bit_begin) {
      do {
	d_ptr -= rot_by_line;	/* happens after case 0 */
      case 7:
	if(*s_ptr & 1<<7) *d_ptr |= d_mask;
	d_ptr -= rot_by_line;
      case 6:
	if(*s_ptr & 1<<6) *d_ptr |= d_mask;
	d_ptr -= rot_by_line;
      case 5:
	if(*s_ptr & 1<<5) *d_ptr |= d_mask;
	d_ptr -= rot_by_line;
      case 4:
	if(*s_ptr & 1<<4) *d_ptr |= d_mask;
	d_ptr -= rot_by_line;
      case 3:
	if(*s_ptr & 1<<3) *d_ptr |= d_mask;
	d_ptr -= rot_by_line;
      case 2:
	if(*s_ptr & 1<<2) *d_ptr |= d_mask;
	d_ptr -= rot_by_line;
      case 1:
	if(*s_ptr & 1<<1) *d_ptr |= d_mask;
	d_ptr -= rot_by_line;
      case 0:
	if(*s_ptr & 1<<0) *d_ptr |= d_mask;
      } while(--s_ptr > s_ptr_begin);
    }
    
    if(++d_bit & 1<<3) {
      d_bit = 0;
      d_ptr_begin++;
    }
  } while(s_ptr >= bitmap);

  return rot_bitmap;
}

/* Lookup table to reverse the bits in a byte */
unsigned char reverse_byte[256] = {
  0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,
  0xf0,0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,
  0x78,0xf8,0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,
  0xb4,0x74,0xf4,0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,
  0x3c,0xbc,0x7c,0xfc,0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,
  0xd2,0x32,0xb2,0x72,0xf2,0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,
  0x5a,0xda,0x3a,0xba,0x7a,0xfa,0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,
  0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,
  0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,
  0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,0x09,0x89,0x49,0xc9,0x29,0xa9,
  0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,0x05,0x85,0x45,0xc5,0x25,
  0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,0x0d,0x8d,0x4d,0xcd,
  0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,0x03,0x83,0x43,
  0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,0x0b,0x8b,
  0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,0x07,
  0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
  0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,
  0xff
};

/*
** Puts a copy of pic rotated 180 degrees into rot_pic.
*/
Byte *rotate_pic_180(pic,rot_pic,bitmap)
     Picture *pic,*rot_pic;
     Byte *bitmap;
{
  Byte *rot_bitmap;
  int num_bytes;
  register Byte *d_ptr;
  register Byte rev,dest;
  register int rshift,lshift;

  rot_pic->width = pic->width;
  rot_pic->height = pic->height;
  rot_pic->offset_x = pic->width - pic->offset_x - 1;
  rot_pic->offset_y = pic->height - pic->offset_y - 1;

  num_bytes = ((pic->width + 7) >> 3) * pic->height;
  rot_bitmap = (Byte *) malloc((unsigned) (num_bytes * sizeof(Byte)));
  
  /*
  ** Scan across each source scanline in the bitmap starting
  ** from the bottom right corner and working left and up.
  ** Reverse the source bytes, shift, and OR into the destination.
  */
  lshift = pic->width & 0x7;
  rshift = 8 - lshift;
  d_ptr = rot_bitmap;

  /* Special case aligned rotation */
  if(lshift == 0) {
    while(--num_bytes >= 0)
      *d_ptr++ = reverse_byte[bitmap[num_bytes]];
  }
  else {
    dest = reverse_byte[bitmap[--num_bytes]] >> rshift;
    while(--num_bytes >= 0) {
      rev = reverse_byte[bitmap[num_bytes]];
      dest |= rev << lshift;
      *d_ptr++ = dest;
      dest = rev >> rshift;
    }
    *d_ptr = dest;
  }

  return rot_bitmap;
}
