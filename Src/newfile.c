/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** file.c
*/

/*
$Author: lidl $
$Id: newfile.c,v 1.1.1.1 1995/02/01 00:25:37 lidl Exp $
*/

#include <string.h>
#include <ctype.h>
#include "tanklimits.h"
#include "malloc.h"
#include <assert.h>
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "vdesc.h"
#include "setup.h"
#include "vehicle.h"
#include "terminal.h"
#include "globals.h"
#include "proto.h"

#ifdef UNIX
#include <sys/param.h>
#ifndef __hpux
#include <sys/dir.h>
#endif
#endif

#if defined(SYSV) || defined(SVR4)
#include <dirent.h>
#endif

#include "vstructs.h"

extern char *mount_entries[];
extern char *side_entries[];

#define check_range(val,mx) \
  if((int)(val) < 0 || (int)(val) >= (int)(mx)) return DESC_BAD_FORMAT;

/*
** Loads the vehicle description from the file "[vehiclesdir]/[name].v".
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT.
*/
new_load_vdesc(d, name)
Vdesc *d;
char *name;
{
	extern char pathname[], vehiclesdir[];
	extern int num_vehicle_objs;
	extern Object *vehicle_obj[];
	FILE *file;
	char filename[MAXPATHLEN];
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
	check_range(d->body, num_vehicle_objs);
	num_tur = vehicle_obj[d->body]->num_turrets;

	(void) fscanf(file, "%d", &d->engine);
	check_range(d->engine, MAX_ENGINES);

	(void) fscanf(file, "%d", &d->num_weapons);
	check_range(d->num_weapons, MAX_WEAPONS + 1);

	for (i = 0; i < d->num_weapons; i++) {
		(void) fscanf(file, "%d %d", &weapon, &mount);
		check_range((int) weapon, MAX_WEAPON_STATS);

		switch (mount) {
		  case MOUNT_TURRET1:
			  if (num_tur < 1)
				  return DESC_BAD_FORMAT;
			  break;
		  case MOUNT_TURRET2:
			  if (num_tur < 2)
				  return DESC_BAD_FORMAT;
			  break;
		  case MOUNT_TURRET3:
			  if (num_tur < 3)
				  return DESC_BAD_FORMAT;
			  break;
		  case MOUNT_TURRET4:
			  if (num_tur < 4)
				  return DESC_BAD_FORMAT;
			  break;
		  case MOUNT_FRONT:
		  case MOUNT_BACK:
		  case MOUNT_LEFT:
		  case MOUNT_RIGHT:
			  break;
		  default:
			  return DESC_BAD_FORMAT;
		}

		d->weapon[i] = weapon;
		d->mount[i] = mount;
	}

	(void) fscanf(file, "%d", &d->armor.type);
	check_range(d->armor.type, MAX_ARMORS);

	for (i = 0; i < MAX_SIDES; i++) {
		(void) fscanf(file, "%d", &d->armor.side[i]);
		if (d->armor.max_side < d->armor.side[i])
			d->armor.max_side = d->armor.side[i];
	}

	(void) fscanf(file, "%d", &d->specials);
	(void) fscanf(file, "%d", &d->heat_sinks);

	(void) fscanf(file, "%d", &d->suspension);
	check_range(d->suspension, MAX_SUSPENSIONS);

	(void) fscanf(file, "%d", &d->treads);
	check_range(d->treads, MAX_TREADS);

	(void) fscanf(file, "%d", &d->bumpers);
	check_range(d->bumpers, MAX_BUMPERS);

	/* Compute parameters, and see if there are any problems */
	if (compute_vdesc(d))
		return DESC_BAD_FORMAT;

	(void) fclose(file);

	return DESC_LOADED;
}

char *Wnames[VMAX_WEAPONS];
char *Mnames[NUM_MOUNTS];
char *Snames[MAX_SIDES];

#define BSZ 80
char *abbrev_of(str)
char *str;
{
	char buf[BSZ];
	char *sp, *bp;

	sp = str - 1;
	bp = buf;

	while (*++sp) {
		if (isupper(*sp) || isdigit(*sp))
			*bp++ = *sp;
	}

	assert(bp - buf < BSZ);
	*bp = '\0';

	bp = strdup(buf);
	assert(bp);
	return bp;
}

void init_Wnames()
{
	int i;

	for (i = 0; i < VMAX_WEAPONS; i++) {
		Wnames[i] = weapon_stat[i].type;
	}

	for (i = 0; i < NUM_MOUNTS; i++) {
		Mnames[i] = abbrev_of(mount_entries[i]);
	}

	for (i = 0; i < MAX_SIDES; i++) {
		Snames[i] = side_entries[i];
	}
}

/*
** Saves the specified vehicle description.
** Returns one of DESC_NOT_FOUND, DESC_SAVED.
*/
int SaveVehicleFormat1(d)
Vdesc *d;
{
	extern char pathname[], vehiclesdir[];
	FILE *file;
	char filename[MAXPATHLEN];
	int i;
	Flag flag;

	if (!Wnames[0])
		init_Wnames();

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

	(void) strcat(filename, d->name);
	(void) strcat(filename, ".V");
	if ((file = fopen(filename, "w")) == NULL)
		return DESC_NOT_FOUND;

	/* Save all the fields in the vdesc structure */
	(void) fprintf(file, "Vehicle: %s\n", d->name);
	(void) fprintf(file, "Designer: %s\n", d->designer);
	(void) fprintf(file, "Body: %s\n", body_stat[d->body].type);
	(void) fprintf(file, "Engine: %s\n", engine_stat[d->engine].type);
	for (i = 0; i < d->num_weapons; i++) {
		(void) fprintf(file, "W#%d: %s %s\n", i,
					   Mnames[d->mount[i]], Wnames[d->weapon[i]]);
	}

	(void) fprintf(file, "Armor Type: %s\n", armor_stat[d->armor.type].type);
	for (i = 0; i < MAX_SIDES; i++) {
		(void) fprintf(file, "%s: %d\n", Snames[i], d->armor.side[i]);
	}

	for (flag = d->specials, i = 0; flag; flag >>= 1, i++) {
		if (flag & 0x1) {
			(void) fprintf(file, "Special: %s\n", special_stat[i].type);
		}
	}
	(void) fprintf(file, "Heat sinks: %d\n", d->heat_sinks);

	(void) fprintf(file, "Suspension: %s\n",
				   suspension_stat[d->suspension].type);
	(void) fprintf(file, "Treads: %s\n", tread_stat[d->treads].type);
	(void) fprintf(file, "Bumpers: %s\n", bumper_stat[d->bumpers].type);

	(void) fclose(file);

#ifdef UNIX
	/* Change protections so others can read the vehicle in */
	(void) chmod(filename, 0644);
#endif

	return DESC_SAVED;
}
