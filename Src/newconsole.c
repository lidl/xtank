/*
** Xtank
**
** Copyright 1992 by Joshua Osborne
**
** $Id$
*/

#include <ctype.h>		/* for isdigit() */
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "vehicle.h"
#include "terminal.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif /* SOUND */


extern Weapon_stat weapon_stat[];
extern Settings settings;

/* Bar graph widths, heights and max values */
#define ARMOR_W  18
#define ARMOR_H  35
#define ARMOR_M  99
#define ARMOR_H2 16
#define ARMOR_M2 42

#define FUEL_W   102
#define FUEL_H   16
#define FUEL_M   1000
#define HEAT_M   100

#define FSCALE   10

static char *console_raw_strs[] =
{
	"    Armor    Player: %1                 ",
	"     %44     Score: %2        Kills:  %3",
	"             Money: %4        Deaths: %5",
	"                                        ",
	"              Vehicle: %6               ",
	" %47 %48 %45    Speed: %7   x: %8 y: %9 ",
	"                       %50              ",
	"     %49                                ",
	"                 Fuel: %10              ",
	"     %46                                ",
	"                 Heat: %11              ",
	"                                        ",
	"              Console: %12   Radar: %13 ",
	"               Mapper: %14  Safety: %15 ",
	"             Teleport: %16  Repair: %17 ",
	"               NewRad: %18 TacLink: %19 ",
	"                                        ",
	"# Weapon              Mt  Ammo  Status  ",
	"1 %20                 %21 %22   %23     ",
	"  %51                                   ",
	"2 %24                 %25 %26   %27     ",
	"  %52                                   ",
	"3 %28                 %29 %30   %31     ",
	"  %53                                   ",
	"4 %32                 %33 %34   %35     ",
	"  %54                                   ",
	"5 %36                 %37 %38   %39     ",
	"  %55                                   ",
	"6 %40                 %41 %42   %43     ",
	"  %56                                   "
};

static char *console_strs[sizeof(console_raw_strs) / sizeof(char *)];

static int inited_strs = False;

static int console_sp_color[] =
{GREY, GREY, WHITE, RED};
static char *console_sp_status[] =
{"---", "off", "on", "brk"};

static int console_sf_color[] =
{GREY, WHITE};
static char *console_sf_status[] =
{"off", "on"};

#ifdef JOSH

static int console_w_color[] =
{RED, RED, GREY, GREEN,
 RED, RED, RED, RED};

static char *console_w_status[] =
{"broken", "broken", "off", "on",
 "no ammo", "no ammo", "no ammo", "no ammo"};	/* Add on-off to noammo */

#else /* JOSH  ie, Aaron */

/*
 *	   	no_am   func    on
 * off & broken	0       0       0       broken, red
 * on & broken	0       0       1       BROKEN, red
 * norm off	0       1       0       off, grey
 * norm on	0       1       1       on, green
 * mt brk off	1       0       0       broken, red
 * mt brk on 	1       0       1       BROKEN, red
 * mt & off	1       1       0       off, red
 * mt & on	1       1       1       no ammo, red
 */

static int console_w_color[] =
{RED, RED, GREY, GREEN, RED, RED, RED, RED};

static char *console_w_status[] =
{"damaged", "DAMAGED", "offline", "ONLINE",
 "damaged", "DAMAGED", "offline", "EMPTY"};


#endif /* JOSH */

static char *console_mount[] =
{"T1", "T2", "T3", "T4", "F", "B", "L", "R"};

static int
idx2armor(int idx, int *sidep)
{
	*sidep = (idx >= 44 && idx <= 47) ? 1 : 0;
	switch (idx) {
	  case 44:
		  return FRONT;
	  case 45:
		  return RIGHT;  /* Switched LEFT and RIGHT (HAK 3/93) */
	  case 46:
		  return BACK;
	  case 47:
		  return LEFT;
	  case 48:
		  return TOP;
	  case 49:
		  return BOTTOM;
	}
}

static void
con_init(Vehicle *v, char *record)
{
	int i, j;
	char *cp;
	int idx;
	crecord *crec = (crecord *) record;
	static int nrec = 0;

	for (i = 0; i < sizeof(console_raw_strs) / sizeof(char *); i++) {
		if (console_strs[i]) {
			/* A bit wasteful of CPU to do this init part for everyone, and I
			** was going to skip it, but you do need to set everyone's .x and
			** .y, so I have to do some of this... */
			free(console_strs[i]);
		}
		assert(console_strs[i] = strdup(console_raw_strs[i]));
		for (cp = console_strs[i]; *cp; cp++) {
			if (*cp == '%') {
				int width;

				*cp = '\0';
				width = font_string_width(console_strs[i], S_FONT);

				*cp++ = ' ';
				assert(idx = atoi(cp));
				assert(idx < MAX_ENTRIES);
				if (idx > nrec)
					nrec = idx;

				crec->item[idx].y = i * font_height(S_FONT) + TOP_BORDER;
				crec->item[idx].x = width + LEFT_BORDER;
				while (isdigit(*cp))
					*cp++ = ' ';
				cp--; /* spl@houston.geoquest.slb.com --ane */
			}
		}
	}

	crec->num = nrec + 1;
	crec->changed = FALSE; /* spl@houston.geoquest.slb.com --ane */

	for (i = 0; i < crec->num; i++) {
		crec->item[i].fval = (float *) NULL;
		crec->item[i].ival = (int *) NULL;
		crec->item[i].action = NULL;
		crec->item[i].red = crec->item[i].green = crec->item[i].white = 0;
		crec->item[i].oldf = -1.0;
		crec->item[i].oldi = -1;

		crec->item[i].changed = False;

		crec->item[i].str_const = NULL;
		crec->item[i].str_array = NULL;
		crec->item[i].color_array = NULL;

		crec->item[i].min = crec->item[i].max = 0;
	}

	crec->item[0].type = CNONE;

	crec->item[1].type = CSTRING_CONST;
	crec->item[1].str_const = v->owner->name;

	crec->item[2].type = CINT;
	crec->item[2].ival = &v->owner->score;

	crec->item[3].type = CINT;
	crec->item[3].ival = &v->owner->kills;

	crec->item[4].type = CINT;
	crec->item[4].ival = &v->owner->money;
	crec->item[4].red = 0;
	crec->item[4].white = v->vdesc->cost;
	crec->item[4].green = 2 * v->vdesc->cost;

	crec->item[5].type = CINT;
	crec->item[5].ival = &v->owner->deaths;

	crec->item[6].type = CSTRING_CONST;
	crec->item[6].str_const = v->name;

	crec->item[7].type = CFLOAT;
	crec->item[7].fval = &v->vector.speed;

	crec->item[50].type = CHFBAR;
	crec->item[50].fval = &v->vector.speed;
	crec->item[50].max = ceil(v->vdesc->max_speed);
	crec->item[50].min = -crec->item[50].max;
	crec->item[50].y += 3;
	crec->item[50].w = 15;
	crec->item[50].h = 1;
	crec->item[50].red = 0;
	crec->item[50].white = 1;
	crec->item[50].green = 1;

	crec->item[8].type = CINT;
	crec->item[8].ival = &v->loc->grid_x;

	crec->item[9].type = CINT;
	crec->item[9].ival = &v->loc->grid_y;

	crec->item[10].type = CHFBAR;
	crec->item[10].fval = &v->fuel;
	crec->item[10].max = v->fuel;
	crec->item[10].w = 15;
	crec->item[10].h = 2;
	crec->item[10].red = 0;
	crec->item[10].white = (v->fuel / 10);
	crec->item[10].green = (v->fuel / 10) * 9;

	crec->item[11].type = CHBAR;
	crec->item[11].ival = &v->heat;
	crec->item[11].max = 100;
	crec->item[11].w = 15;
	crec->item[11].h = 2;
	crec->item[11].red = 100;
	crec->item[11].white = 10;
	crec->item[11].green = 0;

	crec->item[12].type = CSTRING_ARRAY;
	crec->item[12].ival = (int *) &v->special[CONSOLE].status;
	crec->item[12].str_array = console_sp_status;
	crec->item[12].color_array = console_sp_color;

	crec->item[13].type = CSTRING_ARRAY;
	crec->item[13].ival = (int *) &v->special[RADAR].status;
	crec->item[13].str_array = console_sp_status;
	crec->item[13].color_array = console_sp_color;

	crec->item[14].type = CSTRING_ARRAY;
	crec->item[14].ival = (int *) &v->special[MAPPER].status;
	crec->item[14].str_array = console_sp_status;
	crec->item[14].color_array = console_sp_color;

	crec->item[15].type = CSTRING_ARRAY;
	crec->item[15].ival = (int *) &v->safety;
	crec->item[15].str_array = console_sf_status;
	crec->item[15].color_array = console_sf_color;

	crec->item[16].type = CSTRING_ARRAY;
	crec->item[16].ival = (int *) &v->teleport;
	crec->item[16].str_array = console_sf_status;
	crec->item[16].color_array = console_sf_color;

	crec->item[17].type = CSTRING_ARRAY;
	crec->item[17].ival = (int *) &v->special[REPAIR].status;
	crec->item[17].str_array = console_sp_status;
	crec->item[17].color_array = console_sp_color;

	crec->item[18].type = CSTRING_ARRAY;
	crec->item[18].ival = (int *) &v->special[NEW_RADAR].status;
	crec->item[18].str_array = console_sp_status;
	crec->item[18].color_array = console_sp_color;

	crec->item[19].type = CSTRING_ARRAY;
	crec->item[19].ival = (int *) &v->special[TACLINK].status;
	crec->item[19].str_array = console_sp_status;
	crec->item[19].color_array = console_sp_color;

	for (i = 20, j = 0; i < 44; i += 4, j++) {
		Weapon *w = &v->weapon[j];

		crec->item[i + 0].type = crec->item[i + 1].type =
		  crec->item[i + 2].type = crec->item[i + 3].type =
		  crec->item[51 + j].type = CNONE;
		if (j >= v->num_weapons) {
			continue;
		}
		crec->item[i + 0].type = CSTRING_CONST;
		crec->item[i + 0].str_const = weapon_stat[(int) (w->type)].type;

		crec->item[i + 1].type = CSTRING_CONST;
		crec->item[i + 1].str_const = console_mount[w->mount];

		crec->item[i + 2].type = CINT;
		crec->item[i + 2].ival = &w->ammo;
		crec->item[i + 2].red = 0;
		crec->item[i + 2].white = w->ammo / 10;
		crec->item[i + 2].green = (w->ammo / 10) * 9;

		crec->item[i + 3].type = CSTRING_ARRAY;
		crec->item[i + 3].ival = (int *) &w->status;
		crec->item[i + 3].str_array = console_w_status;
		crec->item[i + 3].color_array = console_w_color;

		crec->item[j + 51].type = CHBAR;
		crec->item[j + 51].max = w->ammo;
		crec->item[j + 51].red = 0;
		crec->item[j + 51].white = w->ammo / 10;
		crec->item[j + 51].green = (w->ammo / 10) * 9;
		crec->item[j + 51].ival = &w->ammo;
		crec->item[j + 51].y += 3;
		crec->item[j + 51].w = 30;
		crec->item[j + 51].h = 1;
	}

	for (i = 44; i <= 49; i++) {
		int idx = idx2armor(i, &j);

		crec->item[i].type = CVBAR;
		crec->item[i].max = v->vdesc->armor.max_side;
		crec->item[i].red = 0;
		crec->item[i].white = (v->armor.side[idx] / 10) * 2;
		crec->item[i].green = (v->armor.side[idx] / 10) * 9;
		crec->item[i].ival = &v->armor.side[idx];
		crec->item[i].w = 3;
		crec->item[i].h = j ? 4 : 2;
	}

	for (i = 0; i < crec->num; i++) {
		if (crec->item[i].type != CVBAR && crec->item[i].type != CHBAR
			&& crec->item[i].type != CHFBAR) {
			crec->item[i].h = font_height(S_FONT);
			continue;
		}
		crec->item[i].h *= font_height(S_FONT);
		crec->item[i].w *= font_string_width("W", S_FONT);
		if (crec->item[i].type == CVBAR) {
			crec->item[i].w -= font_string_width("W", S_FONT) / 2;
			crec->item[i].h -= font_height(S_FONT) / 2;
		} else {
			crec->item[i].h -= font_height(S_FONT) / 2;
		}
	}
}

SpecialStatus
special_dummy(Vehicle *v, char *record, int action)
{
	return SP_on;
}

SpecialStatus
special_console(Vehicle *v, char *record, int action)
{
	crecord *crec = (crecord *) record;
	int i;
	cstat *item;

	switch (action) {
	  case SP_update:
		  while (crec->item[1].type != CSTRING_CONST) {
			  printf("Had to kludge plr %s's console\n", v->owner->name);
			  con_init(v, crec);/* Kludge */
		  }

		  for (i = 0; i < crec->num; i++) {
			  switch (crec->item[i].type) {
				case CFLOAT:
				case CHFBAR:
					if (*crec->item[i].fval != crec->item[i].oldf) {
						crec->item[i].changed = TRUE;
						crec->changed++;
					}
					break;
				case CNONE:
				case CSTRING_CONST:
					if (crec->item[i].changed) {
						crec->changed++;
					}
					break;
				case CINT:
				case CSTRING_ARRAY:
				case CVBAR:
				case CHBAR:
					if (*crec->item[i].ival != crec->item[i].oldi) {
						crec->item[i].changed = TRUE;
						crec->changed++;
					}
					break;
				default:
					assert(0 /* bad crec type */ );
			  }
		  }
		  break;
	  case SP_redisplay:
		  if (crec->changed == 0) {
			  break;
		  }
		  for (i = 0; i < crec->num && crec->changed; i++) {
			  char *str, buf[40];
			  int color;

			  if (!crec->item[i].changed) {
				  continue;
			  }
			  crec->changed--;

			  item = crec->item + i;
			  item->changed = False;

			  str = NULL;
			  color = WHITE;

			  /* 2 switches, to group common code.  A macro would also do
				* and be just as efficent */
			  if (item->white) {
				  switch (item->type) {
					case CINT:
					case CVBAR:
					case CHBAR:
						if (item->white < item->red) {
							/* green < white < red */
							color = (*item->ival >= item->red) ? RED :
							  (*item->ival < item->white) ? GREEN :
							  WHITE;
						} else {
							/* red < white < green */
							color = (*item->ival >= item->green) ? GREEN :
							  (*item->ival < item->white) ? RED :
							  WHITE;
						}
						break;
					case CFLOAT:
					case CHFBAR:
						if (item->white < item->red) {
							/* green < white < red */
							color = (*item->fval >= item->red) ? RED :
							  (*item->fval < item->white) ? GREEN :
							  WHITE;
						} else {
							/* red < white < green */
							color = (*item->fval >= item->green) ? GREEN :
							  (*item->fval < item->white) ? RED :
							  WHITE;
						}
						break;
					 default:
						break;
				  }
			  }
			  switch (item->type) {
				case CINT:
					str = buf;
					sprintf(str, "%d", *item->ival);
					item->oldi = *item->ival;
					break;
				case CFLOAT:
					str = buf;
					sprintf(str, "%.1f", *item->fval);
					item->oldf = *item->fval;
					break;
				case CNONE:
					break;
				case CSTRING_CONST:
					str = item->str_const;
					break;
				case CSTRING_ARRAY:
					str = *(item->str_array + *item->ival);
					if (item->color_array) {
						color = *(item->color_array + *item->ival);
					}
					item->oldi = *item->ival;
					break;
				case CVBAR:
				case CHBAR:
#ifdef SOUND
					console_sound(v, i, item->old_color, color);
#endif /* SOUND */
					display_bar(CONS_WIN, item->x, item->y,
								item->w, item->h,
								*item->ival, &item->old_color, color,
								item->min, item->max,
								item->type == CVBAR, FALSE);
					item->oldi = *item->ival;
					break;
				case CHFBAR:
#ifdef SOUND
					console_sound(v, i, item->old_color, color);
#endif /* SOUND */
					display_bar(CONS_WIN, item->x, item->y,
								item->w, item->h,
					  (int) (FSCALE * *item->fval), &item->old_color, color,
								FSCALE * item->min, FSCALE * item->max,
								FALSE, FALSE);
					item->oldf = *item->fval;
					break;
				default:
					assert(0 /* Bad item type! */ );
			  }
			  if (str) {
#ifdef SOUND
				  console_sound(v, i, item->old_color, color);
#endif /* SOUND */
				  draw_filled_rect(CONS_WIN,
					  item->x, item->y, item->w, item->h, DRAW_COPY, BLACK);
				  draw_text_left(CONS_WIN, item->x, item->y,
								 str, S_FONT, DRAW_COPY, color);
				  item->w = font_string_width(str, S_FONT);
			  }
		  }
		  break;

	  case SP_draw:
		  /* Draw the text template */
		  for (i = 0; i < sizeof(console_raw_strs) / sizeof(char *); i++) {
			  draw_text_rc(CONS_WIN, 0, i, console_strs[i],
						   S_FONT, WHITE);
		  }
		  for (i = 0; i < crec->num; i++) {
			  item = crec->item + i;
			  if (item->type == CNONE) {
				  continue;
			  }
			  if (item->type == CHBAR || item->type == CVBAR) {
				  display_bar(CONS_WIN, item->x, item->y,
							  item->w, item->h,
							  *item->ival, &item->old_color, item->old_color,
							  item->min, item->max,
							  item->type == CVBAR, TRUE);
			  } else {
				  if (item->type == CHFBAR) {
					  display_bar(CONS_WIN, item->x, item->y,
								  item->w, item->h,
					   (int) *item->fval, &item->old_color, item->old_color,
								  item->min, item->max,
								  item->type == CVBAR, TRUE);
				  } else {
					  if (!item->changed) {
						  item->changed = TRUE;
						  crec->changed++;
					  }
				  }
			  }
			  special_console(v, record, SP_redisplay);
		  }
		  break;
	  case SP_erase:
		  clear_window(CONS_WIN);
		  break;
	  case SP_activate:
		  con_init(v, crec);
		  return SP_on;
		  break;
	  case SP_deactivate:
		  break;
	}
}

/*
** Puts a bar on the screen in the specified location, spec'ed color
** spec'ed direction.  Sets last_color.  Note doesn't care if min != 0,
** but *should*
*/
static void
display_bar(int w, int x, int y, int width, int height, int val,
	int *last_color, int new_color, int min, int max,
	Boolean vert, Boolean init)
{
	int filled;

	if (val > max) {
		val = max;
	}
	if (val < min) {
		val = min;
	}
	if (max > 0) {
		if (vert)
			filled = (height - 2) * val / max;
		else
			filled = (width - 2) * val / max;
	} else {
		filled = 0;
	}

	/* Draw the outline rectangle on initialization */
	if (init || *last_color != new_color) {
		draw_rect(w, x, y, width - 1, height - 1, DRAW_COPY, new_color);
	}
	*last_color = new_color;

	/* Only erase the rest of the bar area during non-initialization */
	if (vert) {
		draw_filled_rect(w, x + 1, y + height - 1 - filled, width - 2, filled,
						 DRAW_COPY, new_color);
		if (!init)
			draw_filled_rect(w, x + 1, y + 1, width - 2, height - 2 - filled,
							 DRAW_COPY, BLACK);
	} else {
		draw_filled_rect(w, x + 1, y + 1, filled, height - 2, DRAW_COPY,
						 new_color);
		if (!init)
			draw_filled_rect(w, x + 1 + filled, y + 1, width - 2 - filled,
							 height - 2, DRAW_COPY, BLACK);
	}
}

#ifdef SOUND
#define _MIN_AMMO_INDEX_	51
#define _MAX_AMMO_INDEX_	56
#define _MIN_ARMOR_INDEX_	44 
#define _MAX_ARMOR_INDEX_	49
#define _HEAT_INDEX_		10
#define _FUEL_INDEX_		11

/*
 * console plays sounds depending on color changes
 */
console_sound(v, i, old_color, new_color)
Vehicle	*v;
int	i;
int	old_color;
int	new_color;
{
	/*
	 * warning condition
	 */
	if (new_color == RED && old_color != RED)
	{
		if (i >= _MIN_AMMO_INDEX_ && i <= _MAX_AMMO_INDEX_)
			play_owner(v, AMMO_WARNING_SOUND);
		else if (i >= _MIN_ARMOR_INDEX_ && i <= _MAX_ARMOR_INDEX_)
			play_owner(v, ARMOR_WARNING_SOUND);
		else if (i == _HEAT_INDEX_)
			play_owner(v, HEAT_WARNING_SOUND);
		else if (i == _FUEL_INDEX_)
			play_owner(v, FUEL_WARNING_SOUND);
	}
	/*
	 * stable condition
	 */
	else if (new_color == WHITE && old_color == RED)
	{
		if (i >= _MIN_AMMO_INDEX_ && i <= _MAX_AMMO_INDEX_)
			play_owner(v, AMMO_OK_SOUND);
		else if (i >= _MIN_ARMOR_INDEX_ && i <= _MAX_ARMOR_INDEX_)
			play_owner(v, ARMOR_OK_SOUND);
		else if (i == _HEAT_INDEX_)
			play_owner(v, HEAT_OK_SOUND);
		else if (i == _FUEL_INDEX_)
			play_owner(v, FUEL_OK_SOUND);
	}
}
#endif /* SOUND */
