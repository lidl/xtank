#include <stdio.h>
#include "xtank.h"
#include "vdesc.h"
#include "vstructs.h"

#define vehiclesdir "."

Armor_stat armor_stat[MAX_ARMORS] = {
    /* type           def wgt spc cost */
    {"Steel",          0,  8,  3,  10},
    {"Kevlar",         0,  3,  3,  20},
    {"Hardened Steel", 1,  8,  3,  20},
    {"Composite",      1,  4,  3,  30},
    {"Compound Steel", 2,  8,  3,  40},
    {"Titanium",       2,  5,  3,  70},
    {"Tungsten",       3, 20,  3, 100}
};

Engine_stat engine_stat[MAX_ENGINES] = {
    /* type             power weight space fuel$  fcap   cost */
    {"Small Electric",     50,   100,   20,   5,   200,  1500},
    {"Medium Electric",   100,   150,   30,   5,   300,  2200},
    {"Large Electric",    200,   200,   40,   5,   400,  3000},
    {"Super Electric",    300,   250,   50,   5,   500,  6000},

    {"Small Combustion",  300,   400,  200,   8,   200,  2000},
    {"Medium Combustion", 400,   500,  300,   8,   300,  2500},
    {"Large Combustion",  500,   600,  400,   8,   400,  3000},
    {"Super Combustion",  600,  1000,  600,   8,   500,  4000},

    {"Small Turbine",     600,  1000,  800,  10,   350,  4000},
    {"Medium Turbine",    700,  1200, 1000,  10,   450,  5000},
    {"Large Turbine",     800,  1500, 1500,  10,   550,  7000},
    {"Turbojet Turbine", 1000,  2000, 2000,  10,   750, 10000},

    {"Fuel Cell",        1200,  1000,  400,  20,   600, 15000},
    {"Fission",          1500,  3000, 3500,  15,  1000, 20000},
    {"Breeder Fission",  1800,  3500, 4000,  15,  1250, 25000},
    {"Fusion",           2250,  4000, 2500,   5,  1500, 40000}
};

Body_stat body_stat[MAX_BODIES] = {
    /* type     size weight wghtlim space  drag hndl trts cost */
    {"Lightcycle", 2,   200,   800,   600,  .10,  8,  0,  3000},
    {"Hexo",       3,  1500,  5000,  4000,  .25,  6,  1,  4000},
    {"Spider",     3,  2500,  8000,  3000,  .40,  7,  1,  5000},
    {"Psycho",     4,  5000, 18000,  8000,  .60,  4,  1,  5000},
    {"Tornado",    4,  7000, 22000, 12000,  .80,  4,  1,  7000},
    {"Marauder",   5,  9000, 28000, 18000, 1.00,  3,  2, 10000},
    {"Tiger",      6, 11000, 35000, 22000, 1.50,  5,  1, 12000},
    {"Rhino",      7, 12000, 40000, 30000, 2.00,  3,  2, 10000},
    {"Medusa",     7, 14000, 40000, 25000, 1.20,  4,  3, 15000},
    {"Malice",     5,  4000, 20000, 15000,  .40,  7,  1, 17000},
    {"Trike",      2,   400,  1600,  1200,  .15,  6,  0,  4000}
};

Suspension_stat suspension_stat[MAX_SUSPENSIONS] = {
    /* type  hndl cost */
    {"Light", -1,  100},
    {"Normal", 0,  200},
    {"Heavy",  1,  400},
    {"Active", 2, 1000}
};

Bumper_stat bumper_stat[MAX_BUMPERS] = {
    /* type  elast   cost */
    {"None",    0.0,    0},
    {"Normal", 0.07,  200},
    {"Rubber", 0.15,  400},
    {"Retro",  0.25, 1000}
};

Heat_sink_stat heat_sink_stat = {500, 1000, 500};

Special_stat special_stat[MAX_SPECIALS] = {
#define QQ(sym,type,cost) {type,cost},
#include "special-defs.h"	/* read this file for an explanation */
#undef  QQ
};

Weapon_stat weapon_stat[VMAX_WEAPONS] = {
#define QQ(sym,type,dam,rng,ammo,tm,spd,wgt,spc,fr,ht,ac,cost) \
    {type,dam,rng,ammo,tm,spd,wgt,spc,fr,ht,ac,cost},
#include "weapon-defs.h"	/* read this file for an explanation */
#undef QQ
};

Tread_stat tread_stat[MAX_TREADS] = {
#define QQ(sym,type,fric,cost) {type,fric,cost},
#include "tread-defs.h"		/* read this file for an explanation */
#undef QQ
};


static char *main_entries[] = {
    "Body", "Engine", "Weapons", "Armor type", "Armor values", "Specials",
    "Heat sinks", "Suspension", "Treads", "Bumpers", "Load vehicle",
"Save vehicle", "Reset vehicle", "Quit"};

static char *mount_entries[] = {
"Turret 1", "Turret 2", "Turret 3", "Front", "Back", "Left", "Right"};

static char *side_entries[] = {
"Front", "Back", "Left", "Right", "Top", "Bottom", "All"};

static char *weapnum_entries[] = {"1", "2", "3", "4", "5", "6"};


main(argc, argv)
int	argc;
char	**argv;
{
	int	i;
	Vdesc	vehicle;

	for (i = 1; i < argc; i++)
	{
		load_vdesc(&vehicle, argv[i]);
		compute_vdesc(&vehicle);
		print_vdesc(&vehicle);
	}
}


/*
** Loads the vehicle description from the file "[vehiclesdir]/[name].v".
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT.
*/
load_vdesc(d, name)
Vdesc *d;
char *name;
{
    extern int num_vehicle_objs;
    extern Object *vehicle_obj[];
    FILE *file;
    char filename[1024];
    int num_tur;
    WeaponType weapon;
    MountLocation mount;
    int i;

    /* Open the vehicle description file */

#ifdef UNIX
    (void) strcpy(filename, pathname);
    (void) strcat(filename, "/");
    (void) strcat(filename, vehiclesdir);
    (void) strcat(filename, "/");
#endif /* UNIX */

#ifdef AMIGA
    (void) strcpy(filename, "XVDIR:");
#endif /* AMIGA */

    (void) strcat(filename, name);
    (void) strcat(filename, ".v");
    if ((file = fopen(filename, "r")) == NULL)
	return DESC_NOT_FOUND;

    /* Initialize max side to 0 */
    d->armor.max_side = 0;

    /* Load the values into the vdesc structure, checking their validity */
    (void) fscanf(file, "%s", d->name);
    (void) fscanf(file, "%s", d->designer);

    (void) fscanf(file, "%d", &d->body);

    (void) fscanf(file, "%d", &d->engine);

    (void) fscanf(file, "%d", &d->num_weapons);

    for (i = 0; i < d->num_weapons; i++)
    {
	(void) fscanf(file, "%d %d", &weapon, &mount);

	d->weapon[i] = weapon;
	d->mount[i] = mount;
    }

    (void) fscanf(file, "%d", &d->armor.type);

    for (i = 0; i < MAX_SIDES; i++)
    {
	(void) fscanf(file, "%d", &d->armor.side[i]);
	if (d->armor.max_side < d->armor.side[i])
	    d->armor.max_side = d->armor.side[i];
    }

    (void) fscanf(file, "%d", &d->specials);
    (void) fscanf(file, "%d", &d->heat_sinks);

    (void) fscanf(file, "%d", &d->suspension);

    (void) fscanf(file, "%d", &d->treads);

    (void) fscanf(file, "%d", &d->bumpers);

    /* Compute parameters, and see if there are any problems */
    if (compute_vdesc(d))
	return DESC_BAD_FORMAT;

    (void) fclose(file);

    return DESC_LOADED;
}


print_vdesc(v)
Vdesc	*v;
{
	int	i;
	
	printf("Name: %s\n", v->name);
	printf("Designer: %s\n", v->designer);
	printf("Body: %-20s Engine: %-25s Heat Sinks: %d\n", body_stat[v->body].type,
			engine_stat[v->engine].type, v->heat_sinks);
	printf("Suspension: %-14s Treads: %-10s Bumpers: %-10s\n", 
			suspension_stat[v->suspension].type,
			tread_stat[v->treads].type,
			bumper_stat[v->bumpers].type);
	printf("Cost: $%-10d Weight: %-6d/%-6d Space: %-5d/%-5d\n", v->cost,
			v->weight, body_stat[v->body].weight_limit,
			v->space, body_stat[v->body].space);
	printf("Max Speed: %-7.2f Tread Accel: %-7.2f Engine Accel: %-7.2f\n", 
			v->max_speed, v->tread_acc, v->engine_acc);
	printf("Handling: %d\n", v->handling);
	printf("# Weapon             Mount\n");
	for (i = 0; i < v->num_weapons; ++i)
		printf("%1d %-18s %s\n", i + 1, weapon_stat[(int) v->weapon[i]].type,
		       mount_entries[(int) v->mount[i]]);
	printf("Armour: %s\n", armor_stat[v->armor.type].type);
	for (i = 0; i < MAX_SIDES; ++i)
		printf("  %-6s  %d\n", side_entries[i], v->armor.side[i]);
	printf("Specials: \n");
	for (i = 0; i < MAX_SPECIALS; i++)
		if (v->specials & (1 << i))
			printf("          %s\n", special_stat[i].type);
}

/*
** Computes the vehicle's parameters from the vehicle description.
** Returns problems flag.
*/
compute_vdesc(d)
    Vdesc *d;
{
    int total_armor, size;
    int i;

    int problems = 0;
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

    for (i = 0; i < d->num_weapons; i++) {
	d->weight += weapon_stat[(int) d->weapon[i]].weight;
	d->space += weapon_stat[(int) d->weapon[i]].space;
	d->cost += weapon_stat[(int) d->weapon[i]].cost;
	if (IS_TURRET(d->mount[i]) &&
		(int) d->mount[i] + 1 > body_stat[d->body].turrets)
	    problems |= BAD_MOUNT;
    }

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
