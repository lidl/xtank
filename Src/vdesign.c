
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** vdesign.c
*/

/*
$Author: lidl $
$Id: vdesign.c,v 1.1.1.1 1995/02/01 00:25:38 lidl Exp $
*/

#include "malloc.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "menu.h"
#include "terminal.h"
#include "vehicleparts.h"
#include "bullet.h"
#include "proto.h"

extern Terminal *term;

#define DISPLAY_ROW   0
#define MENU_ROW      20
#define INPUT_ROW     34
#define LEV0_X   10
#define LEV0_Y   420
#define LEV1_X   170

#ifdef S1024x864
#define VEH_X     600
#define VEH_Y     330
#define VEHICLE_SIZE  70

#define VDESIGN_FONT  L_FONT
#endif

#ifdef S640x400
#define VEH_X     150
#define VEH_Y     75
#define VEHICLE_SIZE  40
#define VDESIGN_FONT  S_FONT
#endif


#define vprint(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,VDESIGN_FONT))

#define dprint(str,condition,y,x) \
  if(status == ON) \
    vprint(str,x,DISPLAY_ROW+y); \
  else if(condition) { \
    clear_text_rc(ANIM_WIN,x,y,30 - x%30,1,VDESIGN_FONT); \
    vprint(str,x,DISPLAY_ROW+y); \
  }

#define dprintn(str,y,x) \
  if(status == ON) \
    vprint(str,x,DISPLAY_ROW+y); \
  else { \
    clear_text_rc(ANIM_WIN,x,y,30 - x%30,1,VDESIGN_FONT); \
    vprint(str,x,DISPLAY_ROW+y); \
  }

#define clear_vdesign_input() {\
  if (input_drawn) { \
  input_drawn = FALSE; \
    clear_text_rc(ANIM_WIN,0,INPUT_ROW,80,8,VDESIGN_FONT); \
    clear_text_rc(ANIM_WIN,0,INPUT_ROW+1,80,8,VDESIGN_FONT); \
  } \
}

#define clear_vdesign_display() \
  clear_text_rc(ANIM_WIN,0,DISPLAY_ROW,80,MENU_ROW-DISPLAY_ROW,VDESIGN_FONT)

/* Values for all the menus used */
#define MAIN_MENU         0
#define ENGINES_MENU      1
#define WEAPONS_MENU      2
#define ARMORS_MENU       3
#define BODIES_MENU       4
#define MOUNTS_MENU       5
#define SPECIALS_MENU     6
#define SUSPENSIONS_MENU  7
#define TREADS_MENU       8
#define BUMPERS_MENU      9
#define SIDES_MENU        10
#define WEAPNUMS_MENU     11
#define MAX_MENUS         12

/* Many changes to armor_stat, engine_stat, and body_stat (HAK and MEL 2/93) */
Armor_stat armor_stat[MAX_ARMORS] =
{
	/* type           def wgt spc cost */
	{"Steel",          0,  5,  3,  10},
	{"Kevlar",         0,  3,  3,  20},
	{"Hardened Steel", 1,  8,  3,  20},
	{"Composite",      1,  4,  3,  30},
	{"Carapice",       1,  7,  2,  80},
	{"Porcelain",      1,  2,  5,  80},
	{"Compound Steel", 2,  8,  3,  40},
	{"Titanium",       2,  5,  3,  70},
	{"Tungsten",       3,  8,  3, 120},
};

Engine_stat engine_stat[MAX_ENGINES] =
{
	/* type             power weight space fuel$  fcap   cost */
	{"Small Electric",     50,   100,   20,   5,    50,  1500},
	{"Medium Electric",   100,   150,   30,   5,   100,  2200},
	{"Large Electric",    200,   200,   40,   5,   200,  3000},
	{"Super Electric",    300,   250,   50,   5,   300,  6000},

	{"Small Combustion",  300,   400,  200,   8,    50,  2000},
	{"Medium Combustion", 400,   500,  300,   8,   100,  2500},
	{"Large Combustion",  500,   600,  400,   8,   200,  3000},
	{"Super Combustion",  600,  1000,  600,   8,   300,  4000},

	{"Small Turbine",     600,  1000,  800,  10,   350,  4000},
	{"Medium Turbine",    700,  1200, 1000,  10,   450,  5000},
	{"Large Turbine",     800,  1500, 1500,  10,   550,  7000},
	{"Turbojet Turbine", 1000,  2000, 2000,  10,   750, 10000},

	{"Fuel Cell",        1500,  1000,  400,  20,   400, 45000},
	{"Fission",          2000,  3000, 3500,  15,   800, 20000},
	{"Breeder Fission",  2500,  3500, 4000,  15,  1050, 25000},
	{"Fusion",          10000, 15000,18000,   5,  1300,100000}
};

Body_stat body_stat[MAX_BODIES] =
{
	/* type     size weight wghtlim space  drag hndl trts cost */
	{"Lightcycle", 2,   200,   800,   600,  .10,  8,  0,  3000},
	{"Trike",      2,   400,  1600,  1200,  .15,  6,  0,  4000},
	{"Hexo",       3,  1500,  5000,  4000,  .25,  6,  1,  4000},
	{"Spider",     3,  2500,  8000,  3000,  .40,  7,  1,  5000},
	{"Psycho",     4,  5000, 18000,  8000,  .60,  4,  1,  5000},
	{"Tornado",    4,  7000, 22000, 12000,  .80,  4,  1,  7000},
	{"Marauder",   5,  9000, 28000, 18000, 1.00,  3,  2, 10000},
	{"Tiger",      6, 11000, 35000, 22000, 1.50,  5,  1, 12000},
	{"Rhino",      7, 15000, 55000, 35000, 2.00,  3,  2, 10000},
	{"Medusa",     7, 14000, 40000, 25000, 1.20,  4,  3, 15000},
	{"Delta",      6, 10000, 20000, 18000,  .15,  6,  2, 14000},
	{"Disk",       7, 15000, 35000, 25000,  .15,  6,  2, 15000},
	{"Malice",     5,  4000, 20000, 15000,  .40,  7,  1, 17000},
	{"Panzy",      8, 22000, 80000, 50000, 3.00,  3,  4, 25000},
};

Suspension_stat suspension_stat[MAX_SUSPENSIONS] =
{
 /* type  hndl cost */
	{"Light", -1, 100},
	{"Normal", 0, 200},
	{"Heavy", 1, 400},
	{"Active", 2, 1000}
};

Bumper_stat bumper_stat[MAX_BUMPERS] =
{
 /* type  elast   cost */
	{"None", 0.0, 0},
	{"Normal", 0.07, 200},
	{"Rubber", 0.15, 400},
	{"Retro", 0.25, 1000}
};

Heat_sink_stat heat_sink_stat =
{500, 1000, 500};

Special_stat special_stat[MAX_SPECIALS] =
{
#define QQ(sym,type,cost) {type,cost},
#include "special-defs.h"		/* read this file for an explanation */
#undef  QQ
};

void creat_harm(void *, void *, Angle);
void creat_mort(void *, void *, Angle);

/*void display_something();*/

void update_seeker(void *);
void update_harm(void *);
void update_mortar(void *);

void hit_blast(int, void *, int, int, void *, void *, void *);

Weapon_stat weapon_stat[VMAX_WEAPONS] =
{
#define QQ(sym,type,dam,rng,ammo,tm,spd,wgt,spc,mspc,fr,ht,ac,cost, \
	   refl,safety,hgt) \
	{type,dam,rng,ammo,tm,spd,wgt,spc,mspc,fr,ht,ac,cost,refl, \
	safety,hgt,

#define RR(n_views,mnt,o_flgs,creat_flgs,disp_flgs,move_flgs,hit_flgs,cr_f, \
	di_f,upd_f,hit_f) \
	n_views,mnt,o_flgs,creat_flgs,disp_flgs,move_flgs,hit_flgs, \
	cr_f, di_f, upd_f, hit_f},

#include "weapon-defs.h"		/* read this file for an explanation */
#undef QQ
#undef RR
};

Tread_stat tread_stat[MAX_TREADS] =
{
#define QQ(sym,type,fric,cost) {type,fric,cost},
#include "tread-defs.h"			/* read this file for an explanation */
#undef QQ
};


static char *main_entries[] =
{
	"Body", "Engine", "Weapons", "Armor type", "Armor values", "Specials",
	"Heat sinks", "Suspension", "Treads", "Bumpers", "Load vehicle",
	"Save vehicle", "Reset vehicle", "Quit"};

char *mount_entries[] =
{
	"Turret 1", "Turret 2", "Turret 3", "Turret 4",
	"Front", "Back", "Left", "Right"};

char *side_entries[] =
{
	"Front", "Back", "Left", "Right", "Top", "Bottom", "All"};

char *engine_title = "Engines             power weight space fuel$ fuelcap   cost";
char *weapon_title = "   Weapons          dam rng ammo tm saf spd  wgt spc mtsp heat ammo$ cost flags hit  mnt";
char *armor_title = "Armors          hitsoff weight space cost";
char *body_title = "Bodies      weight wghtlim  space drag hndl trts   cost";
char *special_title = "Specials       cost";
char *suspension_title = "Suspension hndl cost";
char *tread_title = "Treads     fric   cost";
char *bumper_title = "Bumpers     elas   cost";

static char *weapnum_entries[] =
{"1", "2", "3", "4", "5", "6"};

static char
 *engine_entries[MAX_ENGINES], *weapon_entries[VMAX_WEAPONS + 1], *armor_entries[MAX_ARMORS],
 *body_entries[MAX_BODIES], *special_entries[MAX_SPECIALS], *suspension_entries[MAX_SUSPENSIONS],
 *tread_entries[MAX_TREADS], *bumper_entries[MAX_BUMPERS];

static Menu_int menu_sys;
static whichlevel[MAX_MENUS] =
{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

static Vdesc design_vdesc;
static unsigned int problems;
static Boolean modified = FALSE;/* if the vehicle has been modified
					   since the last save */

static Boolean input_drawn = FALSE;	/* spl@houston.geoquest.slb.com */

/*
** Allows user to design a vehicle.
*/
design_vehicle()
{
	Vdesc *d;

	/* Initialize everything */
	d = &design_vdesc;
	clear_window(ANIM_WIN);
	init_vdesign_interface();
	init_vdesc(d);		/* relocated this (HAK 3/93) */
	vdesign_specials_hil(d);
	vdesign_interface(d);
}

/*
** Initializes the vdesign interface menus.
*/
init_vdesign_interface()
{
	menu_sys_window(&menu_sys, ANIM_WIN);

	menu_norm_make(&menu_sys, MAIN_MENU, "Vdesign", 14, 0,
				   LEV0_X, LEV0_Y, main_entries, M_FONT);
	menu_norm_make(&menu_sys, ENGINES_MENU, engine_title, MAX_ENGINES, 0,
				   LEV1_X, LEV0_Y, engine_entries, M_FONT);
	menu_norm_make(&menu_sys, WEAPONS_MENU, weapon_title, VMAX_WEAPONS, 0,
					 LEV1_X, LEV0_Y, weapon_entries, S_FONT);
	menu_norm_make(&menu_sys, ARMORS_MENU, armor_title, MAX_ARMORS, 0,
				   LEV1_X, LEV0_Y, armor_entries, L_FONT);
	menu_norm_make(&menu_sys, BODIES_MENU, body_title, MAX_BODIES, 0,
				   LEV1_X, LEV0_Y, body_entries, M_FONT);
	menu_norm_make(&menu_sys, MOUNTS_MENU, "Mounts", NUM_MOUNTS, 0,
				   LEV1_X, LEV0_Y, mount_entries, L_FONT);
	menu_flag_make(&menu_sys, SPECIALS_MENU, special_title, MAX_SPECIALS, 0,
				   LEV1_X, LEV0_Y, special_entries, L_FONT);
	menu_norm_make(&menu_sys, SUSPENSIONS_MENU, suspension_title,
				   MAX_SUSPENSIONS, 0, LEV1_X, LEV0_Y,
				   suspension_entries, L_FONT);
	menu_norm_make(&menu_sys, TREADS_MENU, tread_title, MAX_TREADS, 0,
				   LEV1_X, LEV0_Y, tread_entries, L_FONT);
	menu_norm_make(&menu_sys, BUMPERS_MENU, bumper_title, MAX_BUMPERS, 0,
				   LEV1_X, LEV0_Y, bumper_entries, L_FONT);
	menu_norm_make(&menu_sys, SIDES_MENU, "Sides", 7, 0,
				   LEV1_X, LEV0_Y, side_entries, L_FONT);
	menu_norm_make(&menu_sys, WEAPNUMS_MENU, "Number", MAX_WEAPONS, 0,
				   LEV1_X, LEV0_Y, weapnum_entries, L_FONT);
}

/*
** Sets up the correct highlighting on the specials menu
*/
vdesign_specials_hil(d)
Vdesc *d;
{
	int i;

	menu_unhighlight(&menu_sys, SPECIALS_MENU);
	for (i = 0; i < MAX_SPECIALS; i++)
		if (d->specials & (1 << i))
			menu_set_hil(&menu_sys, SPECIALS_MENU, i);
}

/*
** Prompts user for vehicle name, loads that vehicle into the specified vdesc.
*/
void vdesign_load(d)
Vdesc *d;
{
	char name[80];

	input_drawn = TRUE; /* spl@ */
	if (modified && !confirm(ANIM_WIN, "overload this vehicle", 0, INPUT_ROW,
							 VDESIGN_FONT))
		return;
	clear_vdesign_input();

	input_drawn = TRUE; /* spl@ */
	input_string(ANIM_WIN, "Enter vehicle name:", name, 0, INPUT_ROW,
				 VDESIGN_FONT, 256);
	if (name[0] == '\0') {
		clear_vdesign_input();
	} else {
		if (load_vdesc(d, name) == DESC_LOADED) {
			vprint("Vehicle loaded.", 0, INPUT_ROW + 1);
			vdesign_specials_hil(d);
			modified = FALSE;
		} else
			vprint("Error.  Vehicle not loaded.", 0, INPUT_ROW + 1);
	}
}

/*
** Checks if vehicle can be saved, if so, asks for a name, and saves
** the vehicle description.
*/
void vdesign_save(d)
Vdesc *d;
{
	char name[80];

	input_drawn = TRUE; /* spl@ */
	if (!modified) {
		vprint("No changes to save.", 0, INPUT_ROW);
		return;
	}
	if (problems) {
		vprint("Vehicle cannot be saved since it has problems.", 0, INPUT_ROW);
		return;
	}
	input_string(ANIM_WIN, "Enter vehicle name:", name, 0,
				 INPUT_ROW, VDESIGN_FONT, 256);
	if (name[0] == '\0') {
		clear_vdesign_input();
	} else {
		(void) strncpy(d->name, name, MAX_STRING);
		d->name[MAX_STRING - 1] = '\0';
		if (save_vdesc(d) == DESC_SAVED) {
			vprint("Vehicle saved.", 0, INPUT_ROW + 1);
			interface_set_desc(VDESC, name);
			modified = FALSE;
		} else
			vprint("Vehicle name already used.  Try another.",
				   0, INPUT_ROW + 1);
	}
}

/*
** Displays menus and prompts for all the parts of a vehicle.
*/
vdesign_interface(d)
Vdesc *d;
{
	Event ev;
	char temp[256];
	int numev, menu, choice, row, i;
	int weapnum, itemp;
	WeaponType weaptype;
	Boolean quit = FALSE;
	Boolean changed = FALSE;

	weapnum = 0;
	clear_window(ANIM_WIN);
	compute_vdesc(d);
	display_vdesc(d, ON);
	menu_display(&menu_sys, MAIN_MENU);

	do {
		/* Redisplay the current vehicle parameters if they've changed */
		if (changed) {
			compute_vdesc(d);
			display_vdesc(d, REDISPLAY);
			changed = FALSE;
		}
		if (win_exposed(ANIM_WIN)) {
			clear_window(ANIM_WIN);
			display_vdesc(d, ON);
			menu_system_expose(&menu_sys);
			expose_win(ANIM_WIN, FALSE);
		}
		numev = 1;
		get_events(&numev, &ev);
		if (numev < 1)
			continue;
		switch (ev.type) {
		  case EVENT_LBUTTON:
		  case EVENT_MBUTTON:
		  case EVENT_RBUTTON:
			  clear_vdesign_input();

			  menu = menu_hit(&menu_sys, ev.x, ev.y);
			  if (menu == MENU_NULL)
				  break;

			  /* Erase all equal or higher level menus */
			  erase_vdesign_menus(menu);

			  /* Find out which choice on the menu was selected */
			  menu_hit_p(&menu_sys, &ev, &menu, &choice, &itemp);

			  switch (menu) {
				case MAIN_MENU:
					switch (choice) {
					  case 0:
						  menu_disphil(&menu_sys, BODIES_MENU, d->body);
						  break;
					  case 1:
						  menu_disphil(&menu_sys, ENGINES_MENU, d->engine);
						  break;
					  case 2:
						  menu_resize(&menu_sys, WEAPNUMS_MENU,
									  MIN(d->num_weapons + 1, MAX_WEAPONS));
						  menu_display(&menu_sys, WEAPNUMS_MENU);
						  break;
					  case 3:
						  menu_disphil(&menu_sys, ARMORS_MENU, d->armor.type);
						  break;
					  case 4:
						  menu_display(&menu_sys, SIDES_MENU);
						  break;
					  case 5:
						  menu_display(&menu_sys, SPECIALS_MENU);
						  break;
					  case 6:
						  input_drawn = TRUE; /* spl@ */
						  (void) sprintf(temp, "Heat sink weight:%4d  space:%4d  cost:%4d",
										 heat_sink_stat.weight,
										 heat_sink_stat.space,
										 heat_sink_stat.cost);
						  vprint(temp, 0, INPUT_ROW);

						  d->heat_sinks = input_int(ANIM_WIN, "Number of heat sinks",
										 0, INPUT_ROW + 1, d->heat_sinks, 0,
													99, VDESIGN_FONT);
						  modified = changed = TRUE;
						  menu_unhighlight(&menu_sys, MAIN_MENU);
						  clear_vdesign_input();
						  break;
					  case 7:
						  menu_disphil(&menu_sys, SUSPENSIONS_MENU, d->suspension);
						  break;
					  case 8:
						  menu_disphil(&menu_sys, TREADS_MENU, d->treads);
						  break;
					  case 9:
						  menu_disphil(&menu_sys, BUMPERS_MENU, d->bumpers);
						  break;
					  case 10:
						  vdesign_load(d);
						  changed = TRUE;
						  menu_unhighlight(&menu_sys, MAIN_MENU);
						  break;
					  case 11:
						  vdesign_save(d);
						  changed = TRUE;
						  menu_unhighlight(&menu_sys, MAIN_MENU);
						  break;
					  case 12:
						  input_drawn = TRUE; /* spl@ */
						  if (!modified ||
							  confirm(ANIM_WIN, "reset vehicle", 0,
									  INPUT_ROW, VDESIGN_FONT)) {
							  init_vdesc(d);
							  changed = TRUE;
							  modified = FALSE;
						  }
						  menu_unhighlight(&menu_sys, MAIN_MENU);
						  clear_vdesign_input();
						  break;
					  case 13:
						  input_drawn = TRUE; /* spl@ */
						  if (!modified ||
							  confirm(ANIM_WIN, "quit", 0, INPUT_ROW, VDESIGN_FONT))
							  quit = TRUE;
						  menu_unhighlight(&menu_sys, MAIN_MENU);
						  clear_vdesign_input();
						  break;
					}
					break;
				case ENGINES_MENU:
					d->engine = choice;
					modified = changed = TRUE;
					break;
				case WEAPONS_MENU:
					menu_erase(&menu_sys, WEAPONS_MENU);
					if (choice < VMAX_WEAPONS - 1) {
						/* Get mount for added weapon */
						weaptype = (WeaponType) choice;
						menu_display(&menu_sys, MOUNTS_MENU);
					} else {
						/* Delete this weapon */
						if (weapnum != d->num_weapons) {
							d->weapon[weapnum] =
							  d->weapon[--d->num_weapons];
							modified = changed = TRUE;
						}
						menu_unhighlight(&menu_sys, MAIN_MENU);
					}
					break;
				case ARMORS_MENU:
					d->armor.type = choice;
					modified = changed = TRUE;
					break;
				case BODIES_MENU:
					d->body = choice;
					modified = changed = TRUE;
					break;
				case MOUNTS_MENU:
					menu_erase(&menu_sys, MOUNTS_MENU);
					if (weapnum == d->num_weapons)
						++d->num_weapons;
					d->weapon[weapnum] = weaptype;
					d->mount[weapnum] = (MountLocation) choice;
					modified = changed = TRUE;
					menu_unhighlight(&menu_sys, MAIN_MENU);
					break;
				case SPECIALS_MENU:
					d->specials ^= 1 << choice;
					modified = changed = TRUE;
					break;
				case SUSPENSIONS_MENU:
					d->suspension = choice;
					modified = changed = TRUE;
					break;
				case TREADS_MENU:
					d->treads = choice;
					modified = changed = TRUE;
					break;
				case BUMPERS_MENU:
					d->bumpers = choice;
					modified = changed = TRUE;
					break;
				case SIDES_MENU:
				        input_drawn = TRUE; /* spl@ */
					row = INPUT_ROW;
					for (i = 0; i < MAX_SIDES; i++)
						if (choice == MAX_SIDES || choice == i) {
							(void) sprintf(temp, "%s armor value",
										   side_entries[i]);
							d->armor.side[i] =
							  input_int(ANIM_WIN, temp, 0, row++,
									 d->armor.side[i], MIN_ARMOR, MAX_ARMOR,
										VDESIGN_FONT);
						}
					modified = changed = TRUE;
					menu_erase(&menu_sys, SIDES_MENU);
					menu_unhighlight(&menu_sys, MAIN_MENU);
					clear_vdesign_input();
					break;
				case WEAPNUMS_MENU:
					weapnum = choice;
					menu_erase(&menu_sys, WEAPNUMS_MENU);
					if (weapnum < d->num_weapons)
						menu_set_hil(&menu_sys, WEAPONS_MENU, d->weapon[weapnum]);
					menu_display(&menu_sys, WEAPONS_MENU);
					break;
			  }
		}
	} while (quit == FALSE);
}

/*
** Erases all menus on equal or higher levels than the specified menu.
*/
erase_vdesign_menus(mu)
int mu;
{
	int level, i;

	level = whichlevel[mu];

	/* Erase any other displayed menu that is of equal or higher level */
	for (i = 0; i < (int) MAX_MENUS; i++)
		if (menu_is_up(&menu_sys, i) && whichlevel[i] >= level && i != mu)
			menu_erase(&menu_sys, i);
}

/*
** Sets the values in the vehicle description to a standard set.
*/
init_vdesc(d)
Vdesc *d;
{
	int i;

	*d->name = '\0';
	strncpy(d->designer, term->player_name, MAX_STRING);

	/* Make sure there is something in the designer slot */
	if (d->designer[0] == '\0')
		(void) strcpy(d->designer, "Unknown");

	d->num_weapons = 0;
	d->engine = 0;
	d->body = 0;
	d->heat_sinks = 0;
	d->suspension = 1;
	d->treads = 1;
	d->bumpers = 1;

	d->armor.type = 0;
	for (i = 0; i < MAX_SIDES; i++)
		d->armor.side[i] = 0;

	d->specials = 0;
	for (i = 0; i < MAX_SPECIALS; i++) {
#ifndef NO_CAMO
#ifndef NO_HUD
		if (i == (int) HUD || i == (int) CONSOLE || i == (int) MAPPER || i == (int) RADAR || i ==
			(int) NEW_RADAR || i == (int) TACLINK || i == (int) CAMO || i == (int) STEALTH || i == (int) RDF)
#else /* !NO_HUD */
		if (i == (int) CONSOLE || i == (int) MAPPER || i == (int) RADAR || i ==
			(int) NEW_RADAR || i == (int) TACLINK || i == (int) CAMO || i == (int) STEALTH || i == (int) RDF)
#endif /* !NO_HUD */
#else /* !NO_CAMO */
		if (i == (int) CONSOLE || i == (int) MAPPER || i == (int) RADAR || i ==
			(int) NEW_RADAR || i == (int) TACLINK)
#endif /* !NO_CAMO */
			d->specials |= (1 << i);
	}
	vdesign_specials_hil(d);  /* HAK 3/93 */
}

/*
** Computes the vehicle's parameters from the vehicle description.
** Returns problems flag.
*/
compute_vdesc(d)
Vdesc *d;
{
	int total_armor, size;
	int i, mnt;
#ifdef NEW_LIMITS
	int sFront, sBack, sLeft, sRight;
	int sTur1, sTur2, sTur3, sTur4, sideSpace;
#endif

	problems = 0;
	total_armor = 0;

	for (i = 0; i < MAX_SIDES; i++)
		total_armor += d->armor.side[i];

	size = body_stat[d->body].size;

	/* Compute weight, space and cost */
	d->weight = body_stat[d->body].weight +
	  engine_stat[d->engine].weight +
	  total_armor * armor_stat[d->armor.type].weight * size +
	  d->heat_sinks * heat_sink_stat.weight;

	d->space = engine_stat[d->engine].space +
	  total_armor * armor_stat[d->armor.type].space * size +
	  d->heat_sinks * heat_sink_stat.space;

	d->cost = body_stat[d->body].cost +
	  engine_stat[d->engine].cost +
	  total_armor * armor_stat[d->armor.type].cost * size +
	  d->heat_sinks * heat_sink_stat.cost +
	  suspension_stat[d->suspension].cost * size +
	  tread_stat[d->treads].cost * size +
	  bumper_stat[d->bumpers].cost * size;

#ifdef NEW_LIMITS
	sFront=sBack=sLeft=sRight=sTur1=sTur2=sTur3=sTur4=0;
#endif
	for (i = 0; i < d->num_weapons; i++) {
#ifdef NEW_LIMITS
		mnt = weapon_stat[(int) d->weapon[i]].mount;
		switch(d->mount[i]) {
			case MOUNT_TURRET1:
				if( !(mnt & M_TURRET) ) problems |= MIS_MOUNT;
				sTur1++;
				break;
			case MOUNT_TURRET2:
				if( !(mnt & M_TURRET) ) problems |= MIS_MOUNT;
				sTur2++;
				break;
			case MOUNT_TURRET3:
				if( !(mnt & M_TURRET) ) problems |= MIS_MOUNT;
				sTur3++;
				break;
			case MOUNT_TURRET4:
				if( !(mnt & M_TURRET) ) problems |= MIS_MOUNT;
				sTur4++;
				break;
			case MOUNT_FRONT:
				if( !(mnt & M_FRONT) ) problems |= MIS_MOUNT;
				sFront += weapon_stat[(int)d->weapon[i]].mount_space;
				break;
			case MOUNT_BACK:
				if( !(mnt & M_BACK) ) problems |= MIS_MOUNT;
				sBack += weapon_stat[(int)d->weapon[i]].mount_space;
				break;
			case MOUNT_LEFT:
				if( !(mnt & M_LEFT) ) problems |= MIS_MOUNT;
				sLeft += weapon_stat[(int)d->weapon[i]].mount_space;
				break;
			case MOUNT_RIGHT:
				if( !(mnt & M_RIGHT) ) problems |= MIS_MOUNT;
				sRight += weapon_stat[(int)d->weapon[i]].mount_space;
				break;
		}
#endif
#ifndef NEW_LIMITS
		if (!(IS_SIDE(d->mount[i])) && (d->weapon[i] == HARM))
			problems |= MIS_MOUNT;
#endif
		d->weight += weapon_stat[(int) d->weapon[i]].weight;
		d->space += weapon_stat[(int) d->weapon[i]].space;
		d->cost += weapon_stat[(int) d->weapon[i]].cost;
		if (IS_TURRET(d->mount[i]) &&
			(int) d->mount[i] + 1 > body_stat[d->body].turrets)
			problems |= NO_TURRETS;
	}

#ifdef NEW_LIMITS
	if (sTur1 > 2 || sTur2 > 2 || sTur3 > 2 || sTur4 > 2)
		problems |= TURRET_FULL;

	sideSpace = MAX(300, body_stat[d->body].space / 4);
	if (sFront > sideSpace)
		problems |= FRONT_FULL;
	if (sBack > sideSpace)
		problems |= BACK_FULL;
	if (sLeft > sideSpace)
		problems |= LEFT_FULL;
	if (sRight > sideSpace)
		problems |= RIGHT_FULL;
#endif
	for (i = 0; i < MAX_SPECIALS; i++)
		if (d->specials & 1 << i) {
			d->cost += special_stat[i].cost;
			if (i == (int) RAMPLATE)
				d->weight += (d->armor.side[(int) FRONT] / 2) *
				  armor_stat[d->armor.type].weight;
		}
	if (d->weight > body_stat[d->body].weight_limit)
		problems |= OVER_WEIGHT;

	if (d->space > body_stat[d->body].space)
		problems |= OVER_SPACE;

	d->handling = body_stat[d->body].handling_base +
	  suspension_stat[d->suspension].handling_adj;

	{
		int power = engine_stat[d->engine].power;
		float drag = body_stat[d->body].drag;
		float friction = tread_stat[d->treads].friction;

		d->max_speed = pow(power / drag, 0.3333) / friction;
		if (d->treads == HOVER_TREAD)
			d->max_speed /= 2;

		d->engine_acc = 16.0 * power / d->weight;
		d->tread_acc = friction * MAX_ACCEL;	/* weight drops out because
						   traction is proportional to
						   weight */
		d->acc = MIN(d->tread_acc, d->engine_acc);
	}

	return (int) problems;
}

/*
** Displays all information about vehicle.
*/
display_vdesc(d, status)
Vdesc *d;
unsigned int status;
{
	extern Object *vehicle_obj[];
	static Vdesc od;
	static unsigned int oproblems;
	Picture *pic;
	char temp[80];
	int i, row, col;
	int wgt_left, spc_left, armor_units_left;

	(void) sprintf(temp, "Name:       %s", d->name);
	dprint(temp, (strcmp(od.name, d->name)), 0, 0);
	(void) sprintf(temp, "Designer:   %s", d->designer);
	dprint(temp, (strcmp(od.designer, d->designer)), 1, 0);
	(void) sprintf(temp, "Body:       %s", body_stat[d->body].type);
	dprint(temp, (od.body != d->body), 2, 0);
	(void) sprintf(temp, "Engine:     %s", engine_stat[d->engine].type);
	dprint(temp, (od.engine != d->engine), 3, 0);
	(void) sprintf(temp, "Heat Sinks: %d", d->heat_sinks);
	dprint(temp, (od.heat_sinks != d->heat_sinks), 4, 0);
	(void) sprintf(temp, "Suspension: %s",
				   suspension_stat[d->suspension].type);
	dprint(temp, (od.suspension != d->suspension), 5, 0);
	(void) sprintf(temp, "Treads:     %s", tread_stat[d->treads].type);
	dprint(temp, (od.treads != d->treads), 6, 0);
	(void) sprintf(temp, "Bumpers:    %s", bumper_stat[d->bumpers].type);
	dprint(temp, (od.bumpers != d->bumpers), 7, 0);

	dprint("Specials:", FALSE, 8, 0);

	for (i = 0; i < MAX_SPECIALS; i++) {
		if (i < (MAX_SPECIALS >> 1)) {
			row = i;
			col = 12;
		} else {
			row = i - (MAX_SPECIALS >> 1);
			col = 20;
		}
		dprint(((d->specials & 1 << i) ? special_stat[i].type : ""),
		  ((od.specials & 1 << i) != (d->specials & 1 << i)), row + 8, col);
		dprintn(((d->specials & 1 << (row + (MAX_SPECIALS >> 1))) ?
				 special_stat[row + (MAX_SPECIALS >> 1)].type : ""),
				row + 8, 20);
	}

	(void) sprintf(temp, "Cost:     $%d", d->cost);
	dprint(temp, (od.cost != d->cost), 0, 31);
	(void) sprintf(temp, "Weight:   %d/%d", d->weight,
				   body_stat[d->body].weight_limit);
	dprint(temp, (od.weight != d->weight || od.body != d->body), 1, 31);
	(void) sprintf(temp, "Space:    %d/%d", d->space,
				   body_stat[d->body].space);
	dprint(temp, (od.space != d->space || od.body != d->body), 2, 31);
	(void) sprintf(temp, "Max Spd:  %.2f", d->max_speed);
	dprint(temp, (od.max_speed != d->max_speed), 3, 31);
	(void) sprintf(temp, "Tread accel: %.2f", d->tread_acc);
	dprint(temp, (od.tread_acc != d->tread_acc), 4, 31);
	(void) sprintf(temp, "Engine accel: %.2f", d->engine_acc);
	dprint(temp, (od.engine_acc != d->engine_acc), 5, 31);
	(void) sprintf(temp, "Handling: %d", d->handling);
	dprint(temp, (od.handling != d->handling), 6, 31);

	dprint("Problems:", FALSE, 7, 31);
	dprint(((problems & OVER_WEIGHT) ? "OVER WEIGHT LIMIT" : ""),
		   ((problems & OVER_WEIGHT) != (oproblems & OVER_WEIGHT)), 7, 41);
	dprint(((problems & OVER_SPACE) ? "OVER SPACE LIMIT" : ""),
		   ((problems & OVER_SPACE) != (oproblems & OVER_SPACE)), 8, 41);
	dprint(((problems & NO_TURRETS) ? "NOT ENOUGH TURRETS" : ""),
		   ((problems & NO_TURRETS) != (oproblems & NO_TURRETS)), 9, 41);
	dprint(((problems & MIS_MOUNT) ? "MISMOUNTED WEAPON" : ""),
		   ((problems & MIS_MOUNT) != (oproblems & MIS_MOUNT)), 10, 41);
#ifdef NEW_LIMITS
	if (problems & (FRONT_FULL|BACK_FULL|LEFT_FULL|RIGHT_FULL|TURRET_FULL)){
		strcpy(temp, "FULL: ");
		strcat(temp, ((problems & FRONT_FULL) ? "F " : "  "));
		strcat(temp, ((problems & BACK_FULL) ? "B " : "  "));
		strcat(temp, ((problems & LEFT_FULL) ? "L " : "  "));
		strcat(temp, ((problems & RIGHT_FULL) ? "R " : "  "));
		strcat(temp, ((problems & TURRET_FULL) ? "T" : ""));
	} else {
		strcpy(temp, "               ");
	}

	dprint(temp, ((problems & FRONT_FULL) != (oproblems & FRONT_FULL)
		   || (problems & BACK_FULL) != (oproblems & BACK_FULL)
		   || (problems & LEFT_FULL) != (oproblems & LEFT_FULL)
		   || (problems & RIGHT_FULL) != (oproblems & RIGHT_FULL)
		   || (problems & TURRET_FULL) != (oproblems & TURRET_FULL)),
		11, 41);
#endif
	dprint("# Weapon             Mount", FALSE, 14, 0);
	for (i = 0; i < d->num_weapons; ++i) {
		(void) sprintf(temp, "%1d %-18s %s", i + 1,
					   weapon_stat[(int) d->weapon[i]].type,
					   mount_entries[(int) d->mount[i]]);
		dprint(temp,
			   ((od.weapon[i] != d->weapon[i]) ||
				(od.mount[i] != d->mount[i]) ||
				(i >= od.num_weapons)),
			   i + 14, 0);
	}

	/* Erase any extra lines if redisplaying */
	if (status == REDISPLAY)
		for (i = d->num_weapons; i < od.num_weapons; i++)
			dprintn("", i + 14, 0);

/*
 * print some valuable armor info - WNW
 */
	wgt_left = (int) (body_stat[d->body].weight_limit - d->weight) /
	  (armor_stat[d->armor.type].weight * body_stat[d->body].size);
	if (armor_stat[d->armor.type].space) {
		spc_left = (int) (body_stat[d->body].space - d->space) /
		  (armor_stat[d->armor.type].space * body_stat[d->body].size);
	} else {
		spc_left = wgt_left;
	}
	armor_units_left = MIN(wgt_left, spc_left);
	(void) sprintf(temp, "Armor Units Left: %d", armor_units_left);
	dprint(temp, TRUE, 12, 31);

	(void) sprintf(temp, "Armor: %s", armor_stat[d->armor.type].type);
	dprint(temp, (od.armor.type != d->armor.type), 13, 31);
	for (i = 0; i < MAX_SIDES; ++i) {
		(void) sprintf(temp, "%-6s  %d", side_entries[i], d->armor.side[i]);
		dprint(temp, (od.armor.side[i] != d->armor.side[i]), i + 14, 33);
	}

	/* Draw a picture of the body inside a box */
	draw_filled_rect(ANIM_WIN, VEH_X - VEHICLE_SIZE / 2,
					 VEH_Y - VEHICLE_SIZE / 2,
					 VEHICLE_SIZE, VEHICLE_SIZE, DRAW_COPY, BLACK);

	pic = &vehicle_obj[d->body]->pic[12];
	draw_picture(ANIM_WIN, VEH_X, VEH_Y, pic, DRAW_COPY, WHITE);

	/* pretty rectangle around it */
	draw_rect(ANIM_WIN, VEH_X - VEHICLE_SIZE / 2, VEH_Y - VEHICLE_SIZE / 2,
			  VEHICLE_SIZE, VEHICLE_SIZE, DRAW_COPY, WHITE);

	/* Remember the vehicle description for next time */
	od = *d;
	oproblems = problems;
}

/*
** Puts the proper strings into the menus.
*/
init_vdesign()
{
	int i;
	char temp[100];
	char *strdup();
	char *flgtmp;

	for (i = 0; i < VMAX_WEAPONS-1; i++) {
		if(weapon_stat[i].hit_flgs & AREA) {
			flgtmp = "A";
		} else {
			if(weapon_stat[i].height == -1)
				flgtmp = "B";
			else if(weapon_stat[i].height > 0)
				flgtmp = "T";
			else flgtmp = " ";
		}
		(void) sprintf(temp, " %-18s %3d %4d %4d %3d %2d %3d %4d %4d %4d %2d %4d %6d %c%c%c%c%c%c %s %c%c%c%c%c ",
					weapon_stat[i].type,
					weapon_stat[i].damage,
					weapon_stat[i].range,
					weapon_stat[i].max_ammo,
					weapon_stat[i].reload_time,
					weapon_stat[i].safety,
					weapon_stat[i].ammo_speed,
					weapon_stat[i].weight,
					weapon_stat[i].space,
					weapon_stat[i].mount_space,
					weapon_stat[i].heat,
					weapon_stat[i].ammo_cost,
					weapon_stat[i].cost,
					((weapon_stat[i].creat_flgs & F_MAP)?'R':(
					 (weapon_stat[i].creat_flgs & F_BOTH)?'B':'-')),
					(weapon_stat[i].creat_flgs & F_NREL)?'N':'-',
					(weapon_stat[i].move_flgs & F_KEYB)?'K':'-',
					(weapon_stat[i].disp_flgs & F_TELE)?'T':'-',
					(weapon_stat[i].move_flgs & F_MINE)?'M':'-',
					(weapon_stat[i].creat_func || weapon_stat[i].disp_func
					|| weapon_stat[i].upd_func)?'S':'-',
					flgtmp,
					(weapon_stat[i].mount & M_TURRET)?'T':'-',
					(weapon_stat[i].mount & M_FRONT)?'F':'-',
					(weapon_stat[i].mount & M_BACK)?'B':'-',
					(weapon_stat[i].mount & M_LEFT)?'L':'-',
					(weapon_stat[i].mount & M_RIGHT)?'R':'-'
					);
		weapon_entries[i] = strdup(temp);
	}
	weapon_entries[VMAX_WEAPONS - 1] = "Remove weapon";

	for (i = 0; i < MAX_ARMORS; i++) {
		(void) sprintf(temp, "%-14s  %7d %6d %5d %4d",
					   armor_stat[i].type,
					   armor_stat[i].defense, armor_stat[i].weight,
					   armor_stat[i].space, armor_stat[i].cost);
		armor_entries[i] = strdup(temp);
	}

	for (i = 0; i < MAX_ENGINES; i++) {
		(void) sprintf(temp, "%-18s  %5d %6d %5d %5d %7d %6d",
					   engine_stat[i].type,
					   engine_stat[i].power, engine_stat[i].weight,
					   engine_stat[i].space, engine_stat[i].fuel_cost,
					   engine_stat[i].fuel_limit, engine_stat[i].cost);
		engine_entries[i] = strdup(temp);
	}

	for (i = 0; i < MAX_SUSPENSIONS; i++) {
		(void) sprintf(temp, "%-8s  %2d %5d",
					   suspension_stat[i].type,
					   suspension_stat[i].handling_adj,
					   suspension_stat[i].cost);
		suspension_entries[i] = strdup(temp);
	}

	for (i = 0; i < MAX_TREADS; i++) {
		(void) sprintf(temp, "%-10s  %4.2f %5d",
					   tread_stat[i].type,
					   tread_stat[i].friction,
					   tread_stat[i].cost);
		tread_entries[i] = strdup(temp);
	}

	for (i = 0; i < MAX_BUMPERS; i++) {
		(void) sprintf(temp, "%-10s  %4.2f %5d",
					   bumper_stat[i].type,
					   bumper_stat[i].elasticity,
					   bumper_stat[i].cost);
		bumper_entries[i] = strdup(temp);
	}

	for (i = 0; i < MAX_BODIES; i++) {
		(void) sprintf(temp, "%-10s  %6d %7d %6d %4.2f %4d %4d %6d",
					   body_stat[i].type,
					   body_stat[i].weight, body_stat[i].weight_limit,
					   body_stat[i].space, body_stat[i].drag,
					   body_stat[i].handling_base, body_stat[i].turrets,
					   body_stat[i].cost);
		body_entries[i] = strdup(temp);
	}

	for (i = 0; i < MAX_SPECIALS; i++) {
		(void) sprintf(temp, "%-12s  %5d",
					   special_stat[i].type, special_stat[i].cost);
		special_entries[i] = strdup(temp);
	}
}
