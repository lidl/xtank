/*
**  Menu Toolkit
**
**  Copyright 1988 by Terry Donahue
**
**  menu.c
*/

#include <stdio.h>
#include "common.h"
#include "graphics.h"
#include "menu.h"

extern char *malloc(),*calloc();

/* This function sets the window for the Menu System
 */
menu_sys_window(menuobj, wdw)
Menu_int *menuobj;
int wdw;
{
   menuobj->wdw=wdw;
}				/* menu_sys_window */

/* This function takes in various data about a given menu, including which
 * menuing "system" it is going to be in and its menuid, a number assigned
 * by the programmer to the menu...
 */
menu_bare_make(menuobj, menuid, title, size, width, xtop, ytop, fntcode)
Menu_int *menuobj;		/* Object holding all menus of a "system" */
int menuid;			/* integer id number that the menu will be */
char *title;			/* Title of menu, or "" if none */
int size;			/* number of elements (choices) in the menu */
int width;			/* a width setting */
int xtop, ytop;			/* upper left x,y coordinates of menu */
int fntcode;			/* font id */
{
   Menu *m;			/* Temporary menu holder */
   int i;			/* Temporary counter for setting menus */

   if ((menuid>=MAXMENU)||(menuid<0))       /* Kill any bogus menuid's */
      rorre("Yo, Larry, trying to make a menuid that's just not kosher");

   if (menuid>=menuobj->nummenus) { /* Keep menus up to date */
     for (i=menuobj->nummenus; i<menuid; i++)
       menuobj->is_up[i]=MENU_DOWN;
     menuobj->nummenus=menuid+1;
   }
   menuobj->is_up[menuid] = MENU_DOWN;
   MAKE_M;
   m->numitem = size;		/* store the parameters of the menu */
   m->xt      = xtop;
   m->yt      = ytop;
   m->items   = NULL;
   m->hil     = (char *) calloc (size+m->titled,sizeof(char));
   m->font    = fntcode;
   m->fh      = font_height(fntcode);
   m->height  = m->fh*(size+m->titled)+1;
   m->width  = width;
   if (m->titled)
      m->title = title;
}

menu_set_fields(menuobj, menuid, val)
Menu_int *menuobj;
int menuid;
int val;
{
   Menu *m;

   MAKE_M;

   m->border  = (val & MENU_BORDER) > 0;
   m->popup   = (val & MENU_POPUP) > 0;
   m->migrate = (val & MENU_MIGRATE) > 0;
   m->hold    = (val & MENU_HOLD) > 0;
   m->leavhil = (val & MENU_LEAVEHIL) > 0;
   m->titled  = (val & MENU_TITLE) > 0;
   m->onehil  = (val & MENU_ONEHIL) > 0;
   m->nwidth   = (val & MENU_WIDTH) > 0;
}

menu_resize(menuobj, menuid, newsize)
Menu_int *menuobj;
int menuid;
int newsize;
{
   Menu *m;

   MAKE_M;
   m->numitem = newsize;
   m->height  = m->fh*(newsize+m->titled)+1;
}

menu_new_width(menuobj, menuid, newwid)
Menu_int *menuobj;
int menuid;
int newwid;
{
   Menu *m;

   MAKE_M;
   m->width = newwid;
}

/* This fuction displays a given menu on the screen..
 * That's all it does... nothing fancy
 */
menu_display(menuobj, menuid)
Menu_int *menuobj;		/* The menu "system" in use */
int menuid;			/* menu identifier */
{
   int longest,i;		/* The longest input line, counter */
   Menu *m;			/* make life happier */

   if ((menuid>=MAXMENU)||(menuid<0))       /* Kill any bogus menuid's */
      rorre("Yo, Larry, trying to display a menuid that's just not kosher");

   MAKE_M;
   if (!m->nwidth) {
      longest = 0;			/* longest string so far is short */

      for (i=0; i<m->numitem; i++)
	if (longest<font_string_width(m->items[i], m->font))
	  longest=font_string_width(m->items[i], m->font);
      if (m->titled)
	if (longest<font_string_width(m->title, m->font))
	  longest=font_string_width(m->title, m->font);

      m->width = longest+5;
   }
   draw_rect(menuobj->wdw, m->xt, m->yt, m->width, m->height, 
	     DRAW_COPY, MENU_BACK);

   if (m->border)
     draw_rect(menuobj->wdw, m->xt, m->yt, m->width, m->height, 
	       DRAW_COPY, MENU_FORE);

   if (m->titled) {
     draw_text(menuobj->wdw, m->xt+(m->width>>1), m->yt+2, 
	       m->title, m->font, DRAW_COPY, MENU_FORE);
     draw_line(menuobj->wdw, m->xt, m->yt+m->fh+1, 
	       m->xt+m->width, m->yt+m->fh+1, DRAW_COPY, MENU_FORE);
   }

   for (i=0; i<m->numitem; i++) {
      draw_text(menuobj->wdw, 
		m->xt + (m->width>>1),                     /* X */
		m->yt + m->fh*(i+m->titled) + 2,           /* Y */
		m->items[i],                               /* The String */
		m->font,
		DRAW_COPY,
		MENU_FORE);
      if (m->hil[i])
         menu_highlight(menuobj->wdw, m, i);
   }
   menuobj->is_up[menuid]=MENU_UP;
   flush_output();
}

/* erases a menu displayed on the screen
 */

menu_erase(menuobj, menuid)
Menu_int *menuobj;
int menuid;
{
   Menu *m;

   if (menuobj->is_up[menuid]==MENU_DOWN)
     return;			/* Abort if menu is not displayed */
   MAKE_M;

   /* Only unhighlight everything if the menu leaves only one thing lit */
   if(m->onehil) menu_unhighlight(menuobj, menuid);
   draw_filled_rect(menuobj->wdw, m->xt, m->yt, m->width+1, m->height+1,
		    DRAW_COPY, MENU_BACK);
   menuobj->is_up[menuid]=MENU_DOWN;
   flush_output();
}

menu_hit_p(menuobj, x, y, menunum, selec)
Menu_int *menuobj;
int x,y;			/* x and y location of the mouse */
int *menunum;			/* menuid will be returned, or Menu_Null */
int *selec;			/* which item was selected, if any */
{
   Menu *m;
   int menuid;

   for (menuid=0; menuid<menuobj->nummenus; menuid++)
     if (menuobj->is_up[menuid]) {
	MAKE_M;
	if ((x>m->xt)&&(x<m->xt+m->width)&&
	    (y>(m->yt+(m->fh*m->titled)))&&(y<m->yt+m->height-1)) {
	   if (m->onehil)	/* PRIOR TO PICK */
	     menu_unhighlight(menuobj, menuid);
	   if (m->hold) {	/* THE PICK */
	     *selec = menu_track_mouse(menuobj->wdw,m,y);
	     if (*selec == -1) { /* track_mouse returns it ~ left screen */
	       *menunum = MENU_NULL;
	       return;
	     }
	   } else *selec = ((y-m->yt)/(m->fh)) - m->titled;
	   *menunum = menuid;
	   if (m->onehil) {	/* ACT ON INPUT */
	      if (!m->hold)
		menu_highlight(menuobj->wdw, m, *selec);
	      m->hil[*selec]=TRUE;
	   } else if (m->leavhil) {
	      if (m->hil[*selec])
		menu_highlight(menuobj->wdw, m, *selec);
	      m->hil[*selec]=(m->hil[*selec])?(FALSE):(TRUE);
	   } else if (m->hold) {
	      menu_highlight(menuobj->wdw, m, *selec);
	   }
	   return;
	}
     }
   *menunum = MENU_NULL;
}

menu_track_mouse(w,m,y)
int w;				/* window going on */
Menu *m;
int y;
{
   int curchoice;		/* The choice the mouse is now on */
   int numev,x,newchoice;
   Event ev;
   int down=TRUE;

   curchoice = (y-m->yt)/(m->fh)-m->titled;

   if (!m->hil[curchoice])
     menu_highlight(w, m, curchoice);

#ifndef X11
   button_up(w,TRUE);
   follow_mouse(w,TRUE);
#endif

   do {
      numev=1;
      get_events(&numev, &ev);
      if (numev==1) {
	 switch(ev.type) {
	 case EVENT_MBUTTONUP:
	 case EVENT_RBUTTONUP:
	 case EVENT_LBUTTONUP:
	    down=FALSE;
	    break;
	 case EVENT_MOVED:
	    y=ev.y;
	    x=ev.x;
	    if ((x>m->xt)&&(x<m->xt+m->width)&&	/* is in menu p */
		(y>(m->yt+(m->fh*m->titled)))&&(y<m->yt+m->height-1)) {
	       if ((newchoice=(((y-m->yt)/(m->fh))-m->titled))==curchoice)
		 break;
	       else {
		  if (!m->hil[curchoice]&&(curchoice!=-1))
		    menu_highlight(w, m, curchoice);
		  if (!m->hil[curchoice=newchoice])
		    menu_highlight(w, m, curchoice);
	       }
	    } else 
	      if (curchoice!= -1) {
		 if (!m->hil[curchoice])
		   menu_highlight(w, m, curchoice);
		 curchoice = -1;
	      }
	    break;
	 default:
	    break;
	 }
      }
   } while (down);

#ifndef X11
   button_up(w,FALSE);
   follow_mouse(w,FALSE);
#endif

   return (curchoice);
}

menu_highlight(w, m, item)	/* Takes item as nth menu item.. not nth field */
int w;
Menu *m;
int item;
{
   draw_filled_rect(w,
		    m->xt+1, m->yt+(m->fh*(item+m->titled))+1, 
		    m->width-1, m->fh,
		    DRAW_XOR, MENU_FORE);
}

menu_system_expose(menuobj)	/* Does NOT clear the screen.... */
Menu_int *menuobj;
{
   int i;

   for (i=0; i<menuobj->nummenus; i++)
     if (menuobj->is_up[i])
       menu_display(menuobj, i);
}

menu_unhighlight(menuobj, menuid)              
Menu_int *menuobj;
int menuid;
{
   int i;
   Menu *m;       

   MAKE_M;

   for (i=0; i<m->numitem; i++)
      if (m->hil[i]) {
         m->hil[i]=FALSE;
         if(menuobj->is_up[menuid]) menu_highlight(menuobj->wdw, m, i);
      }
}

menu_sys_display(menuobj)
Menu_int *menuobj;
{
   int i;

   for (i=0; i<menuobj->nummenus; i++)
	 menu_display(menuobj, i);
}

menu_sys_erase(menuobj)
Menu_int *menuobj;
{
   int i;

   for (i=0; i<menuobj->nummenus; i++)
      if (menuobj->is_up[i])
	 menu_erase(menuobj, i);
}

menu_hit(menuobj, x, y)
Menu_int *menuobj;
int x,y;			/* x and y location of the mouse */
{
   Menu *m;
   int menuid;

   for (menuid=0; menuid<menuobj->nummenus; menuid++)
     if (menuobj->is_up[menuid]) {
	MAKE_M;
	if ((x>m->xt)&&(x<m->xt+m->width)&&
	    (y>(m->yt+(m->fh*m->titled)))&&(y<m->yt+m->height-1))
	   return menuid;
     }
   return MENU_NULL;
}
