/*
**  Menu Toolkit
**
**  Copyright 1988 by Terry Donahue
**
**  menu.c
*/

/*
$Author: lidl $
$Id: menu.c,v 1.1.1.1 1995/02/01 00:25:36 lidl Exp $
*/

#include "sysdep.h"
#include "malloc.h"
#include <stdio.h>
#include "common.h"
#include "graphics.h"
#include "menu.h"
#include "proto.h"

/* This function sets the window for the Menu System
 */
menu_sys_window(menuobj, wdw)
Menu_int *menuobj;
int wdw;
{
	menuobj->wdw = wdw;
}

/* This function takes in various data about a given menu, including which
 * menuing "system" it is going to be in and its menuid, a number assigned
 * by the programmer to the menu...
 */
menu_bare_make(menuobj, menuid, title, size, width, xtop, ytop, fntcode,
			   has_bar, use_escher, center)
Menu_int *menuobj;				/* Object holding all menus of a "system" */
int menuid;						/* integer id number that the menu will be */
char *title;					/* Title of menu, or "" if none */
int size;						/* number of elements (choices) in the menu */
int width;						/* a width setting */
int xtop, ytop;					/* upper left x,y coordinates of menu */
int fntcode;					/* font id */
int has_bar;					/* has scrollbar */
int use_escher;					/* use escher menus */
int center;						/* center text */
{
	Menu *m;					/* Temporary menu holder */
	int i;						/* Temporary counter for setting menus */

	if ((menuid >= MAXMENU) || (menuid < 0))	/* Kill any bogus menuid's */
		rorre("Yo, Larry, trying to make a menuid that's just not kosher");

	if (menuid >= menuobj->nummenus) {	/* Keep menus up to date */
		for (i = menuobj->nummenus; i < menuid; i++)
			menuobj->is_up[i] = MENU_DOWN;
		menuobj->nummenus = menuid + 1;
	}
	menuobj->is_up[menuid] = MENU_DOWN;
	MAKE_M;
	m->has_bar = has_bar;
	m->center = center;
	m->numitem = size;			/* store the parameters of the menu */
	/* m->offset = 0; */
	if (use_escher) {
		m->frame = vid->escher_width;
	} else {
		m->frame = 1;
	}
	m->xt = xtop;
	m->yt = ytop;
	m->items = NULL;
	m->hil = (char *) calloc(size + m->titled, sizeof(char));

	m->font = fntcode;
	m->fh = font_height(fntcode);
	m->height = m->fh * (size) + 3;
	if (m->titled)
		m->title = title;

	m->sbar.win = menuobj->wdw;
	m->sbar.y = m->yt;
	menu_new_width(menuobj, menuid, width);
	m->sbar.h = m->height;
	m->sbar.pos = 0;
	m->sbar.span = m->numitem;
	m->sbar.total = size;

	menu_resize(menuobj, menuid, size);
}

menu_set_fields(menuobj, menuid, val)
Menu_int *menuobj;
int menuid;
int val;
{
	Menu *m;

	MAKE_M;

	m->border = (val & MENU_BORDER) > 0;
	m->popup = (val & MENU_POPUP) > 0;
	m->migrate = (val & MENU_MIGRATE) > 0;
	m->hold = (val & MENU_HOLD) > 0;
	m->leavhil = (val & MENU_LEAVEHIL) > 0;
	m->titled = (val & MENU_TITLE) > 0;
	m->onehil = (val & MENU_ONEHIL) > 0;
	m->nwidth = (val & MENU_WIDTH) > 0;
}

menu_resize(menuobj, menuid, newsize)
Menu_int *menuobj;
int menuid;
int newsize;
{
	Menu *m;

	MAKE_M;
	m->numitem = newsize;
	m->sbar.total = newsize;
	m->sbar.pos = 0;

	if (m->has_bar) {
		m->sbar.span = (newsize > MAXSPAN) ? MAXSPAN : newsize;
		m->height = m->fh * (m->sbar.span);
	} else {
		m->sbar.span = newsize;
		m->height = m->fh * newsize;
	}
	m->sbar.h = m->height;
}

menu_new_width(menuobj, menuid, newwid)
Menu_int *menuobj;
int menuid;
int newwid;
{
	Menu *m;

	MAKE_M;

	m->width = newwid;

	if (m->has_bar) {
		m->width += 2;
		m->sbar.x = m->width + m->xt + 2;
		m->sbar.w = 10;
	}
}

menu_display_internal(menuobj, menuid)
Menu_int *menuobj;				/* The menu "system" in use */
int menuid;						/* menu identifier */
{
	Menu *m;
	int i, top, height;
	int limit;

	MAKE_M;

	if (m->has_bar) {
		draw_scrollbar(&m->sbar);
		top = m->sbar.pos;
		height = m->sbar.span;
	} else {
		top = 0;
		height = m->numitem;
	}

	limit = MIN(height + top, m->numitem);

	for (i = top; i < limit; i++) {
		if (m->center) {
			draw_text(menuobj->wdw,
					  m->xt + (m->width >> 1),	/* X */
					  m->yt + m->fh * (i - top) + 2,	/* Y */
					  m->items[i],	/* The String */
					  m->font,
					  DRAW_COPY,
					  MENU_FORE);
		} else {
			draw_text_left(menuobj->wdw,
						   m->xt,	/* X */
						   m->yt + m->fh * (i - top) + 2,	/* Y */
						   m->items[i],	/* The String */
						   m->font,
						   DRAW_COPY,
						   MENU_FORE);
		}
		if (m->hil[i]) {
			menu_highlight(menuobj->wdw, m, i);
		}
	}
	return;
}

menu_display_frame(menuobj, menuid)
Menu_int *menuobj;				/* The menu "system" in use */
int menuid;						/* menu identifier */
{
	Menu *m;
	int width;

	MAKE_M;

	width = m->width;
	if (m->has_bar) {
		width += m->sbar.w;
	}
	if (m->border) {
		menu_frame(menuobj->wdw,
				   m->xt, m->yt, width, m->height,
				   DRAW_COPY, MENU_FORE, m->frame);
	}
	if (m->titled) {
		draw_rect(menuobj->wdw,
				  m->xt, m->yt - m->fh - (m->border ? m->frame : 2) - 2,
				  width, m->fh + 2,
				  DRAW_COPY, MENU_FORE);
		draw_text(menuobj->wdw, m->xt + (m->width >> 1),
				  m->yt - m->fh - (m->border ? m->frame : 2),
				  m->title, m->font, DRAW_COPY, MENU_FORE);
	}
	return;
}



/* This fuction displays a given menu on the screen..
 * That's all it does... nothing fancy
 */
menu_display(menuobj, menuid)
Menu_int *menuobj;				/* The menu "system" in use */
int menuid;						/* menu identifier */
{
	int longest, i;				/* The longest input line, counter */
	Menu *m;					/* make life happier */

	if ((menuid >= MAXMENU) || (menuid < 0))	/* Kill any bogus menuid's */
		rorre("Yo, Larry, trying to display a menuid that's just not kosher");

	MAKE_M;

	if (menuobj->is_up[menuid] != MENU_UP)	/* background may be corrupt */
		menu_erase(menuobj, menuid);

	if (!m->nwidth) {
		longest = 0;			/* longest string so far is short */

		for (i = 0; i < m->numitem; i++)
			if (longest < font_string_width(m->items[i], m->font))
				longest = font_string_width(m->items[i], m->font);
		if (m->titled)
			if (longest < font_string_width(m->title, m->font))
				longest = font_string_width(m->title, m->font);

		longest += 5;

		if (m->width != longest) {
			menu_new_width(menuobj, menuid, longest);
		}
	}
	menu_display_frame(menuobj, menuid);

	menu_display_internal(menuobj, menuid);

	menuobj->is_up[menuid] = MENU_UP;
	flush_output();
}

/* erases a menu displayed on the screen
 */

menu_erase(menuobj, menuid)
Menu_int *menuobj;
int menuid;
{
	Menu *m;
	int width;


	if (menuobj->is_up[menuid] == MENU_DOWN)
		return;					/* Abort if menu is not displayed */
	MAKE_M;

	width = m->width;
	if (m->has_bar) {
		width += m->sbar.w;
	}
	/* Only unhighlight everything if the menu leaves only one thing lit */
	if (m->onehil) {
		menu_unhighlight(menuobj, menuid);
	}
	draw_filled_rect(menuobj->wdw,
					 m->xt - m->frame, m->yt - m->frame,
					 width + 2 * m->frame + 1,
					 m->height + 2 * m->frame + 1,
					 DRAW_COPY, MENU_BACK);
	if (m->titled) {
		draw_filled_rect(menuobj->wdw,
					  m->xt, m->yt - m->fh - (m->border ? m->frame : 2) - 2,
						 width + 1,
						 m->fh + 3,	/* one greater than draws */
						 DRAW_COPY, MENU_BACK);
	}
	menuobj->is_up[menuid] = MENU_DOWN;
	flush_output();
}

menu_erase_internal(menuobj, menuid)
Menu_int *menuobj;
int menuid;
{
	Menu *m;

	if (menuobj->is_up[menuid] == MENU_DOWN)
		return;					/* Abort if menu is not displayed */
	MAKE_M;

	/* Only unhighlight everything if the menu leaves only one thing lit */
	if (m->onehil)
		menu_unhighlight(menuobj, menuid);
	draw_filled_rect(menuobj->wdw,
					 m->xt + 1, m->yt + 1,
					 m->width - 1, m->height - 1,
					 DRAW_COPY, MENU_BACK);

	flush_output();
}

menu_redraw(menuobj, menuid)
Menu_int *menuobj;
int menuid;
{

#ifdef DEBUG
	(void) fprintf(stderr, "Redrawing menu %d: %s\n",
				   menuid, menuobj->menus[menuid].title);
#endif

	menu_erase(menuobj, menuid);
	menu_display(menuobj, menuid);
	return;
}

int in_sbar(menuobj, menuid, x, y)
Menu_int *menuobj;
int menuid;
int x, y;
{
	Menu *m;
	scrollbar *sbar;

	MAKE_M;

	if (!m->has_bar) {
		return (FALSE);
	}
	sbar = &m->sbar;

	if (y < sbar->y || y > sbar->y + sbar->h) {
		return (FALSE);
	}
	if (x < sbar->x || x > sbar->x + sbar->w) {
		return (FALSE);
	}
	return (TRUE);

}

menu_resolve_coord(menuobj, menuid, y)	/* returns index to real item */
Menu_int *menuobj;
int menuid, y;
{
	int select;					/* which item was selected, if any */
	int slot;					/* index through items *shown* */
	int pos;
	Menu *m;					/* the usual */

	MAKE_M;

	if (m->has_bar) {
		pos = m->sbar.pos;
	} else {
		pos = 0;
	}

	slot = (y - m->yt) / (m->fh);

	if ((slot < -1) || (slot > m->sbar.span)) {
		(void) fprintf(stderr,
					   "menu_selection: (y == %d) resolved to bogus slot %d span = %d, num = %d\n",
					   y, slot, m->sbar.span, m->numitem);
		return MENU_NOSELECT;
	} else {
		select = (slot >= 0 ? slot + pos : slot);
	}
	return select;
}

menu_adjust(menuobj, menuid, ev)
Menu_int *menuobj;
int menuid;
Event *ev;
{
	int delta, new_offset, pos;
	Menu *m;

	MAKE_M;

	if (in_sbar(menuobj, menuid, ev->x, ev->y)) {
		drag_scrollbar(&m->sbar, ev->x, ev->y, Button2);
		menu_redraw(menuobj, menuid);
		return;
	}
	/* new_offset = m->offset; */
	if (m->has_bar) {
		pos = new_offset = m->sbar.pos;
	} else {
		pos = new_offset = 0;
	}

	delta = (ev->y - m->yt) / m->fh;

	if (ev->type == EVENT_RBUTTON)
		delta = 0 - delta;

	new_offset += delta;
	new_offset = MIN(new_offset, m->numitem - m->sbar.span);
	new_offset = MAX(new_offset, 0);

#ifdef DEBUG
	(void) fprintf(stderr, "Adjusting menu offset to %d\n", new_offset);
#endif

	if (new_offset != pos) {
		menu_erase_internal(menuobj, menuid);
		m->sbar.pos = new_offset;
		menu_display_internal(menuobj, menuid);
	} else {
		menu_redraw(menuobj, menuid);
	}
	return;
}

menu_hit_in_border(menuobj, menuid, x, y)
Menu_int *menuobj;
int menuid;
int x, y;
{
	Menu *m;
	int retval = FALSE;
	int width;

	MAKE_M;

	width = m->width;
	if (m->has_bar) {
		width += m->sbar.w;
	}
	if ((x > m->xt) &&
		(x < m->xt + width) &&
		(y > m->yt - (m->titled ? m->fh : 0)) &&
		(y < m->yt + m->height - 1)) {
		retval = TRUE;
	}
	return (retval);
}

menu_hit_p(menuobj, ev, p_menuid, selec, just_scrolled)
Menu_int *menuobj;
Event *ev;
int *p_menuid;					/* menuid will be returned, or Menu_Null */
int *selec;						/* which item was selected, if any */
int *just_scrolled;
{
	Menu *m;
	int x, y, menuid;

/*
	menuid = *p_menuid;

	MAKE_M;
 *
 *  spl@houston.geoquest.slb.com --ane
 */

	x = ev->x;
	y = ev->y;

	for (menuid = 0; menuid < menuobj->nummenus; menuid++)
		if (menuobj->is_up[menuid]) {
			MAKE_M;
			if (menu_hit_in_border(menuobj, menuid, x, y)) {

				if (in_sbar(menuobj, menuid, ev->x, ev->y)) {
					drag_scrollbar(&m->sbar, ev->x, ev->y, ev->type);
					menu_redraw(menuobj, menuid);
					*p_menuid = MENU_NULL;
					*just_scrolled = 1;
					return;
				}
				if (m->onehil)	/* PRIOR TO PICK */
					menu_unhighlight(menuobj, menuid);
				if (m->hold) {	/* THE PICK */
					*selec = menu_track_mouse(menuobj, menuid, y);
					if (*selec == -1) {	/* track_mouse returns it ~ left screen */
						*p_menuid = MENU_NULL;
						return;
					}
				} else
					*selec = menu_resolve_coord(menuobj, menuid, y);
				*p_menuid = menuid;
				if (m->onehil) {/* ACT ON INPUT */
					if (!m->hold)
						menu_highlight(menuobj->wdw, m, *selec);
					m->hil[*selec] = TRUE;
				} else if (m->leavhil) {
					if (m->hil[*selec])
						menu_highlight(menuobj->wdw, m, *selec);
					m->hil[*selec] = (m->hil[*selec]) ? (FALSE) : (TRUE);
				} else if (m->hold) {
					menu_highlight(menuobj->wdw, m, *selec);
				}
				return;
			}
		}
	*p_menuid = MENU_NULL;
}

menu_track_mouse(menuobj, menuid, y)
Menu_int *menuobj;
int menuid, y;
{
	Menu *m;
	int w;
	int curchoice;				/* The choice the mouse is now on */
	int numev, x, newchoice;
	Event ev;
	int down = TRUE;

	MAKE_M;
	w = menuobj->wdw;

	curchoice = menu_resolve_coord(menuobj, menuid, y);

	if (!m->hil[curchoice])
		menu_highlight(w, m, curchoice);

#ifndef X11
	button_up(w, TRUE);
	follow_mouse(w, TRUE);
#endif

	do {
		numev = 1;
		get_events(&numev, &ev);
		if (numev == 1) {
			switch (ev.type) {
			  case EVENT_MBUTTONUP:
			  case EVENT_RBUTTONUP:
			  case EVENT_LBUTTONUP:
				  down = FALSE;
				  break;
			  case EVENT_MOVED:
				  y = ev.y;
				  x = ev.x;
				  if (menu_hit_in_border(menuobj, menuid, x, y)) {
					  if ((newchoice = menu_resolve_coord(menuobj, menuid, y))
						  == curchoice)
						  break;
					  else {
						  if (!m->hil[curchoice] && (curchoice != -1))
							  menu_highlight(w, m, curchoice);
						  if (!m->hil[curchoice = newchoice])
							  menu_highlight(w, m, curchoice);
					  }
				  } else if (curchoice != -1) {
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
	button_up(w, FALSE);
	follow_mouse(w, FALSE);
#endif

	return (curchoice);
}


/* Takes item as nth menu item.. not nth field */

menu_highlight(w, m, item)
int w;
Menu *m;
int item;
{
	if (m->has_bar) {
		item -= m->sbar.pos;
	}
	draw_filled_rect(w,
					 m->xt + 1,
					 m->yt + (m->fh * (item /* - m->titled */ )) + 1,
					 m->width - 1, m->fh,
					 DRAW_XOR, MENU_FORE);
}

menu_system_expose(menuobj)		/* Does NOT clear the screen.... */
Menu_int *menuobj;
{
	int i;

	for (i = 0; i < menuobj->nummenus; i++)
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

	for (i = 0; i < m->numitem; i++)
		if (m->hil[i]) {
			m->hil[i] = FALSE;
			if (menuobj->is_up[menuid])
				menu_highlight(menuobj->wdw, m, i);
		}
}

menu_sys_display(menuobj)
Menu_int *menuobj;
{
	int i;

	for (i = 0; i < menuobj->nummenus; i++)
		menu_display(menuobj, i);
}

menu_sys_erase(menuobj)
Menu_int *menuobj;
{
	int i;

	for (i = 0; i < menuobj->nummenus; i++)
		if (menuobj->is_up[i])
			menu_erase(menuobj, i);
}

menu_hit(menuobj, x, y)
Menu_int *menuobj;
int x, y;						/* x and y location of the mouse */
{
	int menuid;

	for (menuid = 0; menuid < menuobj->nummenus; menuid++)
		if (menuobj->is_up[menuid]) {
			if (menu_hit_in_border(menuobj, menuid, x, y)) {
				return menuid;
			}
		}
	return MENU_NULL;
}
