#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** Needs a rewrite.
**
** status.c
*/

#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "vehicle.h"


#define KILLS     0
#define ARMOR_BAR 1
#define FUEL_BAR  2
#define AMMO_BAR  3

#define BAR_WIDTH   135
#define BAR_HEIGHT  13
#define BAR_SPACING 7

typedef struct
{
	int kills;
	int minarmor;
	int fuel;
	int ammo;
	Vehicle *vehicle;
	Flag dead;
} Vstatus;

extern int num_vehicles;
extern Vehicle *vehicle[];

int num_stati;
Vstatus vstati[MAX_VEHICLES];
int maxarmor, maxammo;
float maxfuel;
float armor_scale, fuel_scale, ammo_scale;

/*
** This function should be called once at the beginning of each battle.
** It computes the maxima and scaling factors for the bar graphs based
** on the armor, fuel, and ammo values of the battling vehicles.
*/
init_status()
{
	Vstatus *vs;
	int i;

	/* Compute initial stati for vehicle */
	for (i = 0; i < num_vehicles; ++i)
		init_vehicle_status(vehicle[i]);

	/* Default values for the maxima to avoid divide by zero errors */
	maxarmor = 1;
	maxfuel = 1.0;
	maxammo = 1;

	/* Compute maximum values over all vehicles */
	for (i = 0; i < num_vehicles; ++i)
	{
		vs = &vstati[i];
        maxarmor = MAX(maxarmor, vs->minarmor);
        maxfuel = MAX(maxfuel, vs->fuel);
        maxammo = MAX(maxammo, vs->ammo);
	}

	num_stati = num_vehicles;

	/* From the maxima, compute scaling factors for each bar graph */
	armor_scale = (float) BAR_WIDTH / (float) maxarmor;
	fuel_scale = (float) BAR_WIDTH / maxfuel;
	ammo_scale = (float) BAR_WIDTH / (float) maxammo;
}

/*
** Initializes the appropriate vstatus structure for the given vehicle.
** Exposes the appropriate status window.
*/
init_vehicle_status(v)
Vehicle *v;
{
	Vstatus *vs;
	extern int num_terminals;

	vs = &vstati[v->number];

	vs->kills = 0;
	vs->minarmor = compute_minarmor(v);
	vs->fuel = (int) v->fuel;
	vs->ammo = compute_totammo(v);
	vs->vehicle = v;
	vs->dead = FALSE;
	/* if (v->number < num_terminals) */
	if (v->owner->num_players)	/* num_players s/b num_terms */
		expose_win(STAT_WIN + v->number, TRUE);
}

/*
** This function handles display for all of the status windows.
*/
display_status(disptype)
unsigned int disptype;
{
	int i;

	/* Update all of the status windows */
	for (i = 0; i < MAX_STAT_WINDOWS; ++i)
		display_status_win(i, disptype);
}

/*
** This function handles display for a particular status window.
*/
display_status_win(num, disptype)
int num;
unsigned int disptype;
{
	int minarmor, totammo;
	Vstatus *vs;
	Vehicle *v;
	int w, i;

	/* Figure out which vehicle to display information about (if any) */
	vs = (Vstatus *) NULL;
	for (i = 0; i < num_vehicles; i++)
	{
		v = vehicle[i];
		if (v->number == num)
		{
			vs = &vstati[num];
			break;
		}
	}
	if (vs == (Vstatus *) NULL)
		return;

	/* Check for being mapped or exposed */
	w = STAT_WIN + num;
	if (!(win_mapped(w)))
		return;
	check_expose(w, disptype);

	if (disptype == ON)
	{
		if (num >= num_stati)
			clear_window(STAT_WIN + num);
		else
			draw_status_from_scratch(v);
	}
	else if (disptype == REDISPLAY)
	{
		if (num >= num_stati)
			return;

		/* If vehicle just died, put a dead symbol on it */
		if (!(v->status & VS_is_alive) && (v->status & VS_was_alive))
		{
			draw_dead_symbol(num);
			vs->dead = TRUE;
		}
		if (vs->kills != v->owner->kills)
			update(KILLS, (int) v->number, &vs->kills, v->owner->kills, FALSE, v->color);

		minarmor = compute_minarmor(v);
		if (vs->minarmor != minarmor)
			update(ARMOR_BAR, (int) v->number, &vs->minarmor, minarmor, FALSE, v->color);

		if (vs->fuel != (int) v->fuel)
			update(FUEL_BAR, (int) v->number, &vs->fuel, (int) v->fuel,
				   FALSE, v->color);

		totammo = compute_totammo(v);
		if (vs->ammo != totammo)
			update(AMMO_BAR, (int) v->number, &vs->ammo, totammo, FALSE, v->color);
	}
}

draw_status_from_scratch(v)
Vehicle *v;
{
	Vstatus *vs;
	int vnum;
	char buffer[20];

	/* Do the bar graph from scratch */
	vnum = v->number;
	clear_window(STAT_WIN + vnum);

	/* Draw all the text into the window */
	(void) sprintf(buffer, "%d", vnum);
	draw_text(STAT_WIN + vnum, 14, 10, buffer, L_FONT, DRAW_COPY, v->color);

	draw_text(STAT_WIN + vnum, 130, 10, v->owner->name, L_FONT, DRAW_COPY, v->color);

	draw_text(STAT_WIN + vnum, 220, 40, "Kills", M_FONT, DRAW_COPY, v->color);

	draw_text(STAT_WIN + vnum, 25, 39, "Armor", M_FONT, DRAW_COPY, v->color);

	draw_text(STAT_WIN + vnum, 25, 59, "Fuel", M_FONT, DRAW_COPY, v->color);

	draw_text(STAT_WIN + vnum, 25, 79, "Ammo", M_FONT, DRAW_COPY, v->color);


	/* Draw the values into the window */
	vs = &vstati[vnum];

	update(KILLS, vnum, (int *) NULL, vs->kills, TRUE, v->color);
	update(ARMOR_BAR, vnum, (int *) NULL, vs->minarmor, TRUE, v->color);
	update(FUEL_BAR, vnum, (int *) NULL, vs->fuel, TRUE, v->color);
	update(AMMO_BAR, vnum, (int *) NULL, vs->ammo, TRUE, v->color);

	/* If the vehicle is dead, draw the dead symbol on the window */
	if (!(v->status & VS_is_alive))
		draw_dead_symbol(vnum);
}

#define KILL_X 220
#define KILL_Y 60

/*
** Displays the new value, either from scratch, or incrementally,
** and updates the old value.
*/
update(section, vnum, old, new, fromscratch, color)
int section;
int vnum;
int *old, new;
unsigned int fromscratch;
int color;
{
	int dispold, dispnew;
	int diff;
	float scale;
	char oldbuf[2], newbuf[2];
	int xoffset, yoffset;

	/* Handle the kills section specially, since it is different from the
	   rest */
	if (section == KILLS)
	{
		if (!fromscratch)
		{
			/* Erase the old number of kills */
			(void) sprintf(oldbuf, "%d", *old);
			draw_text(STAT_WIN + vnum, KILL_X, KILL_Y, oldbuf,
					  XL_FONT, DRAW_XOR, color);

			/* Update the old information */
			*old = new;
		}
		/* Draw the new number of kills */
		(void) sprintf(newbuf, "%d", new);
		draw_text(STAT_WIN + vnum, KILL_X, KILL_Y, newbuf,
				  XL_FONT, DRAW_XOR, color);
	}
	else
	{
		switch (section)
		{
			case ARMOR_BAR:
				scale = armor_scale;
				yoffset = 35;
				break;
			case FUEL_BAR:
				scale = fuel_scale;
				yoffset = 35 + BAR_HEIGHT + BAR_SPACING;
				break;
			case AMMO_BAR:
				scale = ammo_scale;
				yoffset = 35 + 2 * (BAR_HEIGHT + BAR_SPACING);
				break;
		}

		if (!fromscratch)
		{
			/* Compute the old and new displayed values from the values */
			dispold = (int) (scale * (*old));
			dispnew = (int) (scale * new);

			/* Update the old value to the new value */
			*old = new;

			/* If the displayed value has changed, xor the difference
			   rectangle */
            diff = ABS(dispnew - dispold);
			if (diff > 0)
			{
                xoffset = 55 + MIN(dispold, dispnew);
				draw_filled_rect(STAT_WIN + vnum, xoffset, yoffset,
								 diff, BAR_HEIGHT, DRAW_XOR, color);
			}
		}
		else
		{
			xoffset = 55;
			draw_rect(STAT_WIN + vnum, xoffset - 1, yoffset - 1,
					  BAR_WIDTH + 1, BAR_HEIGHT + 1, DRAW_COPY, color);

			dispnew = (int) (scale * new);
			draw_filled_rect(STAT_WIN + vnum, xoffset, yoffset,
							 dispnew, BAR_HEIGHT, DRAW_COPY, color);
		}
	}
}

draw_dead_symbol(num)
int num;
{
	/* Draw a big X through the window to indicate that the vehicle is dead */
	draw_line(STAT_WIN + num, 0, 0, STAT_WIN_WIDTH, STAT_WIN_HEIGHT, DRAW_XOR, WHITE);
	draw_line(STAT_WIN + num, STAT_WIN_WIDTH, 0, 0, STAT_WIN_HEIGHT, DRAW_XOR, WHITE);
}

/*
** Returns the minimum armor of the front, back, left, and right sides.
*/
compute_minarmor(v)
Vehicle *v;
{
	int *side, mn, i;

	side = v->armor.side;
	mn = 9999;

	for (i = 0; i < 4; i++)
		if (side[i] < mn)
			mn = side[i];

	return mn;
}

/*
** Returns the sum of the ammo of all weapons in the specified vehicle.
*/
compute_totammo(v)
Vehicle *v;
{
	int i, total;

	for (i = 0, total = 0; i < v->num_weapons; ++i)
		total += v->weapon[i].ammo;

	return total;
}
