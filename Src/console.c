/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** console.c
*/

/*
$Author: aahz $
$Id: console.c,v 2.14 1992/02/13 04:54:26 aahz Exp $

$Log: console.c,v $
 * Revision 2.14  1992/02/13  04:54:26  aahz
 * moved safety and teleport before the specials in the processing
 * cases.  The macros depend on order!  This must have been written
 * by a below average thinker.
 *
 * Revision 2.13  1992/02/10  05:17:50  senft
 * We re-ordered some of the items which were displayed in the console.
 * The true specials should always(currently) be consecutive and in the
 * same order as in special-defs.h, otherwise there are indexing errors.
 * Moved teleport and safety before the specials(true).
 *
 * Revision 2.12  1992/02/10  04:47:56  lidl
 * checked in so Matt can dick with it
 *
 * Revision 2.11  1992/01/29  08:37:01  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.10  1992/01/29  06:17:27  stripes
 * The still buggy thing is the missing "dummy" entries for specials...
 *
 * Revision 2.9  1992/01/29  03:18:52  lidl
 * added Aaron's patches, but something is still buggy
 *
 * Revision 2.8  1992/01/10  00:18:25  aahz
 * fixed bugs with teleport on/off
 *
 * Revision 2.7  1992/01/06  07:52:49  stripes
 * Changes for teleport
 *
 * Revision 2.6  1991/12/02  09:38:13  stripes
 * Moved the spacing around in the strings for the console.
 * Also fixed for loop limit to be based on sizeof()s so that
 * the loop will not have to be updated again if strings are
 * added(also was hard coded to wrong number.)[Really senft]
 *
 * Revision 2.4  1991/03/25  00:40:13  stripes
 * RS6K patches
 *
 * Revision 2.3  1991/02/10  13:50:16  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:29  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:07  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:13  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:09  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "vehicle.h"


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

#define disp_bar(w,h,max,max2,vert,init,lowend) \
    display_bar(CONS_WIN,word->x,word->y,w,h,\
              c->_entry[num].value,&c->_entry[num].color,\
              max,max2,vert,init,lowend)

#define make_int_entry(val) \
    ((void) sprintf(e[num].string,"%-*d", \
		    console_word[num].len,e[num].value=val), \
     num++)

#define make_bar_entry(val) \
    (e[num].value = val, \
     num++)

#define make_str_entry(val) \
    ((void) sprintf(e[num].string,"%-*s",console_word[num].len,val), \
     num++)

#define update_int_entry(num,val) \
    (val != e[num].value && \
     ((void)sprintf(e[num].string, "%-*d", \
		    console_word[num].len, \
		    e[num].value = val), \
      c->change[c->num_changes++] = num))

#define update_bar_entry(num,val) \
    (val != e[num].value && (e[num].value = val, \
			     c->change[c->num_changes++] = num))

#define update_status_entry(num,val,array,colorar) \
    (val != e[num].value && \
     ((void) sprintf(e[num].string,"%-*s",console_word[num].len, \
		     array[e[num].value = val]), \
      c->change[c->num_changes++] = num, \
      e[num].color = colorar[val]))

static Word console_init[] = {
	{0, 0, 40,  "    Armor    Player:                    "},
	{0, 1, 40,  "             Score:            Kills:   "},
	{0, 2, 40,  "             Money:           Deaths:   "},
	{0, 3, 40,  "                                        "},
	{0, 4, 40,  "              Vehicle:                  "},
	{0, 5, 40,  "                Speed:     x:    y:     "},
	{0, 6, 40,  "                                        "},
	{0, 7, 40,  "                 Fuel:                  "},
	{0, 8, 40,  "                                        "},
	{0, 9, 40,  "                 Heat:                  "},
	{0, 10, 40, "                                        "},
	{0, 11, 40, "              Console:       Radar:     "},
	{0, 12, 40, "               Mapper:      Safety:     "},
	{0, 13, 40, "             Teleport:      Repair:     "},
	{0, 14, 40, "               NewRad:     TacLink:     "},
	{0, 15, 40, "                                        "},
	{0, 16, 40, "# Weapon              Mt  Ammo  Status  "},
	{0, 17, 40, "1                                       "},
	{0, 18, 40, "2                                       "},
	{0, 19, 40, "3                                       "},
	{0, 20, 40, "4                                       "},
	{0, 21, 40, "5                                       "},
	{0, 22, 40, "6                                       "}
};

/* Names for console entries */
#define CONSOLE_player		0
#define CONSOLE_score		1
#define CONSOLE_kills    	2
#define CONSOLE_money		3

#define CONSOLE_vehicle		4
#define CONSOLE_speed		5
#define CONSOLE_coord_x		6
#define CONSOLE_coord_y		7
#define CONSOLE_fuel		8
#define CONSOLE_heat		9

#define CONSOLE_front_armor	10
#define CONSOLE_back_armor	11
#define CONSOLE_left_armor	12
#define CONSOLE_right_armor	13
#define CONSOLE_top_armor	14
#define CONSOLE_bottom_armor	15

#define CONSOLE_safety      16
#define CONSOLE_teleport	17

/********************************************************************/
/* do not place anything between the console and the last entry for */
/* specials these must be consective and in the same order as in    */
/* special-defs.h.  We've agreed, the next hack starts with:        */
/*     rm console.c                                                 */
/********************************************************************/
#define CONSOLE_console		18
#define CONSOLE_mapper		19
#define CONSOLE_radar		20
#define CONSOLE_repair      21
#define CONSOLE_ram		    22	/* These four are dummy entries     */
#define CONSOLE_deep		23	/* They are required, sorta...      */
#define CONSOLE_stealth		24	/* If you want to add things to the */
#define CONSOLE_nav		    25	/* end of special-defs.h that is... */
#define CONSOLE_newradar	26
#define CONSOLE_taclink		27
/* end of specials (things that are bought for a vehicle and listed */
/* in special-defs.h                                                */

#define CONSOLE_deaths		28
#define CONSOLE_weapon		29
#define CONSOLE_mount		30
#define CONSOLE_ammo		31
#define CONSOLE_weapon_status	32	/* GHS incr by one */

/* Number of entries for each weapon */
#define NUM_WEAPON_ENTRIES	4

static Word console_word[] = {
	{21, 0, 19},				/* Player */
	{20, 1, 7},				/* Score */
	{37, 1, 2},				/* Kills */
	{20, 2, 10},				/* Money */

	{23, 4, 19},				/* Vehicle */
	{23, 5, 2},				/* Speed */
	{30, 5, 2},				/* Coord_x */
	{36, 5, 2},				/* Coord_y */

#if defined(_IBMR2) || defined(apollo)
	{143, 90, 0},				/* Fuel */
	{143, 110, 0},				/* Heat */
#else
	{143, 68, 0},				/* Fuel */
	{143, 88, 0},				/* Heat */
#endif
	{35, 18, 0},				/* Front Armor */
	{35, 92, 0},				/* Back Armor */
	{11, 55, 0},				/* Left Armor */
	{59, 55, 0},				/* Right Armor */
	{35, 55, 0},				/* Top Armor */
	{35, 74, 0},				/* Bottom Armor */

	{36, 12, 3},				/* Safety */
	{23, 13, 3},				/* Teleport */
	{23, 11, 3},				/* Console */
	{23, 12, 3},				/* Mapper */
	{36, 11, 3},				/* Radar */
	{36, 13, 3},				/* Repair */

	{0, 0, 0},				/* Ram dummy entry */
	{0, 0, 0},				/* Deep dummy entry */
	{0, 0, 0},				/* Stealth dummy entry */
	{0, 0, 0},				/* Nav dummy entry */

	{23, 14, 3},				/* New Radar */
	{36, 14, 3},				/* TacLink */
	{37,  2, 3},				/* Deaths */

	{2, 17, 18},				/* Weapon 1 */
	{22, 17, 2},				/* Mount */
	{26, 17, 3},				/* Ammo */
	{32, 17, 8},				/* Status */

	{2, 18, 18},				/* Weapon 2 */
	{22, 18, 2},				/* Mount */
	{26, 18, 3},				/* Ammo */
	{32, 18, 8},				/* Status */

	{2, 19, 18},				/* Weapon 3 */
	{22, 19, 2},				/* Mount */
	{26, 19, 3},				/* Ammo */
	{32, 19, 8},				/* Status */

	{2, 20, 18},				/* Weapon 4 */
	{22, 20, 2},				/* Mount */
	{26, 20, 3},				/* Ammo */
	{32, 20, 8},				/* Status */

	{2, 21, 18},				/* Weapon 5 */
	{22, 21, 2},				/* Mount */
	{26, 21, 3},				/* Ammo */
	{32, 21, 8},				/* Status */

	{2, 22, 18},				/* Weapon 6 */
	{22, 22, 2},				/* Mount */
	{26, 22, 3},				/* Ammo */
	{32, 22, 8}				/* Status */
};

static int console_sp_color[] = {GREY, GREY, WHITE, RED};
static char *console_sp_status[] = {"---", "off", "on", "brk"};

static int console_sf_color[] = {GREY, WHITE};
static char *console_sf_status[] = {"off", "on"};

static int console_w_color[] = {RED, RED, GREY, GREEN,
RED, RED, RED, RED};
static char *console_w_status[] = {"broken", "broken", "off", "on",
"no ammo", "no ammo", "no ammo", "no ammo"};

static char *console_mount[] = {"T1", "T2", "T3", "T4", "F", "B", "L", "R"};

/*ARGSUSED*/
special_dummy(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	return;
}

special_console(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	Console *c;
	Entry *e;
	Weapon *w;
	Word *word;
	int num;
	int i, max_value, max2;

	c = (Console *) record;

	switch (action)
	{
		case SP_update:
			/* Initialize the number of changes to zero */
			c->num_changes = 0;
			e = c->_entry;
			num = 0;

			/* Update player values */
			update_int_entry(CONSOLE_score, v->owner->score);
			update_int_entry(CONSOLE_kills, v->owner->kills);
			update_int_entry(CONSOLE_money, v->owner->money);
			update_int_entry(CONSOLE_deaths, v->owner->deaths);

			/* Update vehicle values */
			update_int_entry(CONSOLE_speed, (int) v->vector.speed);
			update_int_entry(CONSOLE_coord_x, v->loc->grid_x);
			update_int_entry(CONSOLE_coord_y, v->loc->grid_y);
			update_bar_entry(CONSOLE_fuel, (int) v->fuel);
			update_bar_entry(CONSOLE_heat, v->heat);

			/* Update armor values */
			for (i = 0; i < MAX_SIDES; i++)
				update_bar_entry(CONSOLE_front_armor + i, v->armor.side[i]);

			/* Update safety value */
			update_status_entry(CONSOLE_safety, v->safety,
								console_sf_status, console_sf_color);

			/* Update teleport value */
			update_status_entry(CONSOLE_teleport, v->teleport,
								console_sf_status, console_sf_color);

			/* Update special values */
			for (i = 0; i < MAX_SPECIALS; i++) {
				if (i == (int)CONSOLE || i == (int)RADAR ||
					i == (int)NEW_RADAR || i == (int)TACLINK ||
					i == (int)MAPPER || i == (int)REPAIR) {
					update_status_entry(CONSOLE_console + i,
						v->special[(int)CONSOLE + i].status,
						console_sp_status, console_sp_color);
				} else {
					++num; /* skip updating for this special */
				}
			}

			/* Update weapon values */
			num = CONSOLE_ammo;
			for (i = 0; i < v->num_weapons; i++)
			{
				w = &v->weapon[i];
				update_int_entry(num, w->ammo);
				update_status_entry(num + 1, w->status,
									console_w_status, console_w_color);
				num += NUM_WEAPON_ENTRIES;
			}
			break;
		case SP_redisplay:
			if (c->num_changes == 0)
				break;
			for (i = 0; i < c->num_changes; i++)
			{
				num = c->change[i];
				word = &console_word[num];
				switch (num)
				{
					case CONSOLE_front_armor:
					case CONSOLE_back_armor:
					case CONSOLE_left_armor:
					case CONSOLE_right_armor:
						switch (num)
						{
							case CONSOLE_front_armor:
                                max2 = v->vdesc->armor.side[(int)FRONT];
								break;
							case CONSOLE_back_armor:
                                max2 = v->vdesc->armor.side[(int)BACK];
								break;
							case CONSOLE_left_armor:
                                max2 = v->vdesc->armor.side[(int)LEFT];
								break;
							case CONSOLE_right_armor:
                                max2 = v->vdesc->armor.side[(int)RIGHT];
								break;
						}

						if (settings.max_armor_scale)
							max_value = v->vdesc->armor.max_side;
						else
						{
							max_value = max2;
						}
                        disp_bar(ARMOR_W, ARMOR_H, max_value, max2, TRUE, FALSE, TRUE);
						break;
					case CONSOLE_top_armor:
					case CONSOLE_bottom_armor:
						if (num == CONSOLE_top_armor)
						{
                            max2 = v->vdesc->armor.side[(int)TOP];
						}
						else
						{
                            max2 = v->vdesc->armor.side[(int)BOTTOM];
						}

						if (settings.max_armor_scale)
							max_value = v->vdesc->armor.max_side;
						else
							max_value = max2;

                        disp_bar(ARMOR_W, ARMOR_H2, max_value, max2, TRUE, FALSE, TRUE);
						break;
					case CONSOLE_fuel:
                        disp_bar(FUEL_W, FUEL_H, ((int) v->max_fuel), ((int) v->max_fuel),
								 FALSE, FALSE, TRUE);
						break;
					case CONSOLE_heat:
                        disp_bar(FUEL_W, FUEL_H, HEAT_M, HEAT_M, FALSE, FALSE, FALSE);
						break;
					default:
						word->str = c->_entry[num].string;
						draw_text_rc(CONS_WIN, word->x, word->y, word->str,
									 S_FONT, c->_entry[num].color);
						break;
				}
			}
			break;
		case SP_draw:
			/* Draw the text template */
			for (i = 0; i < sizeof(console_init)/sizeof(Word); i++)
			{
				word = &console_init[i];
				draw_text_rc(CONS_WIN, word->x, word->y, word->str,
							 S_FONT, WHITE);
			}

			/* Draw all the entries */
			for (num = 0; num < MAX_ENTRIES; num++)
			{
				word = &console_word[num];
/*
* skips placeholder entries
*/
				if (word->x == 0 && word->y == 0) continue;
				switch (num)
				{
					case CONSOLE_front_armor:
					case CONSOLE_back_armor:
					case CONSOLE_left_armor:
					case CONSOLE_right_armor:
						switch (num)
						{
							case CONSOLE_front_armor:
                                max2 = v->vdesc->armor.side[(int)FRONT];
								break;
							case CONSOLE_back_armor:
                                max2 = v->vdesc->armor.side[(int)BACK];
								break;
							case CONSOLE_left_armor:
                                max2 = v->vdesc->armor.side[(int)LEFT];
								break;
							case CONSOLE_right_armor:
                                max2 = v->vdesc->armor.side[(int)RIGHT];
								break;
						}
						if (settings.max_armor_scale)
							max_value = v->vdesc->armor.max_side;
						else
							max_value = max2;
                        disp_bar(ARMOR_W, ARMOR_H, max_value, max2, TRUE, TRUE, TRUE);
						break;
					case CONSOLE_top_armor:
					case CONSOLE_bottom_armor:
						if (num == CONSOLE_top_armor)
						{
                            max2 = v->vdesc->armor.side[(int)TOP];
						}
						else
						{
                            max2 = v->vdesc->armor.side[(int)BOTTOM];
						}
						if (settings.max_armor_scale)
							max_value = v->vdesc->armor.max_side;
						else
							max_value = max2;
                        disp_bar(ARMOR_W, ARMOR_H2, max_value, max2, TRUE, TRUE, TRUE);
						break;
					case CONSOLE_fuel:
                        disp_bar(FUEL_W, FUEL_H, FUEL_M, FUEL_M, FALSE, TRUE, TRUE);
						break;
					case CONSOLE_heat:
                        disp_bar(FUEL_W, FUEL_H, HEAT_M, HEAT_M, FALSE, TRUE, FALSE);
						break;
					default:
						word->str = c->_entry[num].string;
						draw_text_rc(CONS_WIN, word->x, word->y, word->str,
									 S_FONT, c->_entry[num].color);
						break;
				}
			}
			break;
		case SP_erase:
			clear_window(CONS_WIN);
			break;
		case SP_activate:
			/* * Initialize e and num, which are used by the macros *
			   make_str_entry, make_int_entry, and make_bar_entry. */
			e = c->_entry;
			num = 0;

			/* Make the combatant entries */
			make_str_entry(v->owner->name);
			make_int_entry(v->owner->score);
			make_int_entry(v->owner->kills);
			make_int_entry(v->owner->money);

			/* Make the vehicle entries */
			make_str_entry(v->name);
			make_int_entry((int) v->vector.speed);
			make_int_entry(v->loc->grid_x);
			make_int_entry(v->loc->grid_y);
			make_bar_entry((int) v->fuel);
			make_bar_entry(v->heat);

			/* Make the armor entries */
			for (i = 0; i < MAX_SIDES; i++)
				make_bar_entry(v->armor.side[i]);

			/* Make the safety entry */
			make_str_entry(console_sf_status[v->safety]);

			/* Make the teleport entry */
			make_str_entry(console_sf_status[v->teleport]);

			/* Make the specials entries */
			for (i = 0; i < MAX_SPECIALS; i++) {
				if (i == (int)CONSOLE || i == (int)RADAR ||
					i == (int)NEW_RADAR || i == (int)TACLINK ||
					i == (int)MAPPER || i == (int)REPAIR) {
					make_str_entry(console_sp_status[v->special[(int)CONSOLE + i].status]);
					} else {
						num++;
					}
			}

			make_int_entry(v->owner->deaths);

			/* Make the weapon entries */
			for (i = 0; i < v->num_weapons; i++)
			{
				w = &v->weapon[i];
		/* name of weapon */
                make_str_entry(weapon_stat[(int)w->type].type);
                make_str_entry(console_mount[(int)w->mount]);
				make_int_entry(w->ammo);
				make_str_entry(console_w_status[w->status]);
			}

			assert(MAX_ENTRIES >= num-1);

			break;
		case SP_deactivate:
			break;
	}
}

/*
** Puts a bar on the screen in the specified location.
*/
display_bar(w, x, y, width, height, val, last_color, mx, mx2, vert, init,
	    low_end)
int w, x, y, width, height, val, *last_color, mx, mx2;
Boolean vert, init, low_end;
{
	int filled;
	int color = WHITE;

	if (val > mx)
		val = mx;

	if (low_end)
	{
		if (val == mx2)
			color = GREEN;
		else if (val * 5 <= mx)	/* 20 % */
			color = RED;
	}
	else
	{
		if (val >= (mx * 4) / 5)/* 80 % */
			color = RED;
	}


	if (mx > 0)
	{
		if (vert)
			filled = (height - 2) * val / mx;
		else
			filled = (width - 2) * val / mx;
	}
	else
		filled = 0;

	/* Draw the outline rectangle on initialization */
	if (init || *last_color != color)
		draw_rect(w, x, y, width - 1, height - 1, DRAW_COPY, color);

	*last_color = color;

	/* Only erase the rest of the bar area during non-initialization */
	if (vert)
	{
        draw_filled_rect(w, x + 1, y + height - 1 - filled, width - 2, filled,
			 DRAW_COPY, color);
		if (!init)
            draw_filled_rect(w, x + 1, y + 1, width - 2, height - 2 - filled,
			     DRAW_COPY, BLACK);
	}
	else
	{
        draw_filled_rect(w, x + 1, y + 1, filled, height - 2, DRAW_COPY,
			 color);
		if (!init)
            draw_filled_rect(w, x + 1 + filled, y + 1, width - 2 - filled,
			     height - 2, DRAW_COPY, BLACK);
	}
}
