 /* * Toolkit *
 * Copyright 1988 by Terry Donahue *
 * menu.h */

/*
 I've been waiting all night for someone like you.
 That's . . . . . . . . . . . . . . . you'll have to do

                        - "Run Run Run", The Psychedellic Furs, _Forever Now_
*/

/*
$Author: rpotter $
$Id: menu.h,v 2.3 1991/02/10 13:51:21 rpotter Exp $

$Log: menu.h,v $
 * Revision 2.3  1991/02/10  13:51:21  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:38  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:33  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:12  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:53  aahz
 * Initial revision
 * 
*/

#include "scroll.h"

#define MAXMENU      25	/* Max number of menus allowed in a "system" */

#define MENU_NULL MAXMENU+1	/* A null return code */

#define MENU_DOWN     0	/* Fields of char if_up */
#define MENU_UP       1
 /* Foreground and Background colors, for now */
#define MENU_FORE     WHITE
#define MENU_BACK     BLACK

#define MENU_POPUP    (1<<0)	/* For passign parameters to menu fields */
#define MENU_MIGRATE  (1<<1)
#define MENU_BORDER   (1<<2)
#define MENU_HOLD     (1<<3)
#define MENU_LEAVEHIL (1<<4)
#define MENU_TITLE    (1<<5)
#define MENU_ONEHIL   (1<<6)
#define MENU_WIDTH    (1<<7)

#define MENU_NOSELECT    (-127)

typedef struct
{  /* Will hold a menu */
    int   numitem;	/* Number of elements (choices) on menu */
    char **items;	/* The choices (in text) */
    char *title;	/* The menu's title */
	int offset;     /* Number of items offset from item zero CN-K */
	int frame;      /* Delta withing escher frames */
    int   xt, yt;	/* Top left coordinates of the Menu */
    int   width, height;	/* Bottom right coordinates of the Menu */
    int   font, fh;	/* The font code for the menu and its height */
    char *hil;	/* Which fields are highlighted now */
    /* Bitfields (Booleans) */
    unsigned popup:1;	/* popup menu or not */
    unsigned migrate:1;	/* migrateable. i.e. relocateable */
    unsigned border:1;	/* Whether or not the menu has a border */
    unsigned hold:1;	/* mouse button hold down for selection */
    unsigned leavhil:1;	/* Can a field be highlighted and left */
    unsigned titled:1;	/* Title fields? */
    unsigned onehil:1;	/* Only one highlighted at once */
    unsigned nwidth:1;	/* A width set, or dynamically determined */
	unsigned has_bar:1; /* Does the menu have a scroll bar? */
	scrollbar sbar;
} Menu;

/* A Menu_Interface object, will hold the "system" of menus */
typedef struct {
    int   wdw;			/* Window in which the "system" resides */
    Menu  menus[MAXMENU];	/* The actual menus */
    int   nummenus;		/* 1 over the largest menu ID */
    char  is_up[MAXMENU];	/* Whether menu n is visual or not */
} Menu_int;

/* Macros for my Menu Code */
#define MAKE_M  m = &(menuobj->menus[menuid])	/* use this to make things srt
						   */

/* Macros for the programmer using the Menu Toolkit */
#define GET_MENU(m,mid) ((m)->menus[mid])	/* Gets the menu mid in
						   "system" m */

#define menu_set_choices(m,mid,dati) /* Makes the char *dati[] the choices */ \
  ((m)->menus[mid].items = dati)	/* for menu mid of menu system m..... */

#define menu_set_frame(m,mid,dati)  /* Set the frame (border) width */ \
  ((m)->menus[mid].frame = (dati)) /* for menu mid of menu system m..... */

#define menu_complex_make(mo, mi, ti, s, w, x, y, dt, fn, flags) \
    (menu_set_fields(mo, mi, flags), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_norm_make(mo, mi, ti, s, w, x, y, dt, fn) \
    (menu_set_fields(mo, mi, MENU_BORDER | MENU_HOLD | MENU_LEAVEHIL | MENU_TITLE | MENU_ONEHIL), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_noho_make(mo, mi, ti, s, w, x, y, dt, fn, es) \
    (menu_set_fields(mo, mi, \
		     MENU_BORDER | MENU_LEAVEHIL | MENU_TITLE | MENU_ONEHIL), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,es), \
     menu_set_choices(mo,mi,dt))

#define menu_noti_make(mo, mi, ti, s, w, x, y, dt, fn) \
    (menu_set_fields(mo, mi, \
		     MENU_BORDER | MENU_HOLD | MENU_LEAVEHIL | MENU_ONEHIL), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_recv_make(mo, mi, ti, s, w, x, y, dt, fn) \
    (menu_set_fields(mo, mi, \
		     MENU_BORDER | MENU_HOLD | MENU_LEAVEHIL | MENU_TITLE | \
		     MENU_ONEHIL | MENU_WIDTH ), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_scroll_make(mo, mi, ti, s, w, x, y, dt, fn) \
    (menu_set_fields(mo, mi, MENU_BORDER | MENU_HOLD | MENU_TITLE), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,TRUE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_nohil_make(mo, mi, ti, s, w, x, y, dt, fn) \
    (menu_set_fields(mo, mi, MENU_BORDER | MENU_HOLD | MENU_TITLE), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_simp_make(mo, mi, ti, s, w, x, y, dt, fn) \
    (menu_set_fields(mo, mi, MENU_BORDER), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_flag_make(mo, mi, ti, s, w, x, y, dt, fn) \
    (menu_set_fields(mo, mi, \
		     MENU_BORDER | MENU_HOLD | MENU_TITLE | MENU_LEAVEHIL), \
     menu_bare_make(mo,mi,ti,s,w, x,y,fn,FALSE,TRUE), \
     menu_set_choices(mo,mi,dt))

#define menu_set_hil(mo, mi, i) \
      ((mo)->menus[mi].hil[(int)i]=(char)TRUE)

#define menu_disphil(mo, mi, i) \
    (menu_set_hil(mo, mi, i), \
     menu_display(mo, mi))

#define menu_is_up(mo, mi) \
      ((mo)->is_up[mi])
