/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** file.c
*/

/*
$Author: lidl $
$Id: file.c,v 2.7 1991/09/24 14:08:56 lidl Exp $

$Log: file.c,v $
 * Revision 2.7  1991/09/24  14:08:56  lidl
 * changes to reflect moving the .h files in xtank/Src/Include
 *
 * Revision 2.6  1991/09/19  05:29:45  lidl
 * added NONAMETAGS flag ifdefs
 *
 * Revision 2.5  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.4  1991/09/15  06:54:19  stripes
 * *** empty log message ***
 *
 * Revision 2.3  1991/02/10  13:50:29  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:43  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:21  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:24  rpotter
 * small changes
 * 
 * Revision 1.2  90/12/30  02:24:33  aahz
 * added functions for loading setups.  These are only stubs so far.
 * 
 * Revision 1.1  90/12/29  21:02:21  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include <assert.h>
#include "xtank.h"
#include "graphics.h"
#include "mazeconv.c"
#include "gr.h"
#include "vdesc.h"
#include "setup.h"
#ifdef UNIX
#include <sys/param.h>
#include <sys/dir.h>
#endif


#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef MAXNAMLEN
#define MAXNAMLEN 256
#endif

char pathname[MAXPATHLEN];
char headersdir[MAXPATHLEN];	/* full name of directory to find headers in */
char vehiclesdir[MAXNAMLEN], mazesdir[MAXNAMLEN], programsdir[MAXNAMLEN];
char username[MAX_STRING], displayname[256];

#ifdef NEED_AUX_FONT
char fontdir[MAXNAMLEN];
#endif

extern char **vehicles_entries, **mazes_entries, **setups_entries;

Vdesc *vdesc = NULL;
int num_vdescs = 0;

Mdesc *mdesc = NULL;
int num_mdescs = 0;

Sdesc *sdesc = NULL;
int num_sdescs = 0;

/*
** Makes all vehicles listed in [vehiclesdir]/list and all mazes listed in
** [mazesdir]/list.
*/
load_desc_lists()
{
    FILE *file;
    char filename[MAXPATHLEN];
    char name[MAXNAMLEN];
    int num;

    /* Make all the vehicle descriptions in the vehicle list */

#ifdef UNIX
    (void) strcpy(filename, pathname);
    (void) strcat(filename, "/");
    (void) strcat(filename, vehiclesdir);
    (void) strcat(filename, "/");
    (void) strcat(filename, "list");
#endif /* UNIX */

#ifdef AMIGA
    (void) strcpy(filename, "XVDIR:list");
#endif /* AMIGA */

    draw_text_rc(ANIM_WIN, 0, 1, "Reading vehicle list...", M_FONT, WHITE);
    sync_output(TRUE);
    if ((file = fopen(filename, "r")) != NULL)
    {
	while (fscanf(file, "%s", name) != EOF)
	    make_vdesc(name, &num);

	(void) fclose(file);
    } else {
	fprintf(stderr, "Could not open file '%s'.\n", filename);
	exit(1);
    }


    /* Make all the maze descriptions in the maze list */

#ifdef UNIX
    (void) strcpy(filename, pathname);
    (void) strcat(filename, "/");
    (void) strcat(filename, mazesdir);
    (void) strcat(filename, "/");
    (void) strcat(filename, "list");
#endif /* UNIX */

#ifdef AMIGA
    (void) strcpy(filename, "XMDIR:list");
#endif /* AMIGA */

    draw_text_rc(ANIM_WIN, 0, 2, "Reading maze list...", M_FONT, WHITE);
    sync_output(TRUE);
    if ((file = fopen(filename, "r")) != NULL)
    {
        while (fscanf(file, "%s", name) != EOF)
	    make_mdesc(name, &num);

        (void) fclose(file);
    } else {
        fprintf(stderr, "Could not open file '%s'.\n", filename);
        exit(1);
    }
}

/*
** Loads the vdesc from the appropriate file into the next empty slot.
** Puts the number of the vehicle into num, if loaded.
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT, DESC_NO_ROOM.
*/
make_vdesc(name, num)
char *name;
int *num;
{
    Vdesc *d;
    int retval, i;

    /* if (num_vdescs >= MAX_VDESCS) return DESC_NO_ROOM; */

    /* if the vehicle could not be found, we will be leaving room for an
       extra Vdesc.  This should probably be cleaned up later. */

    if (vdesc == NULL)
    {
	vdesc = (Vdesc *) malloc(sizeof(Vdesc));
	vehicles_entries = (char **) malloc(sizeof(char *));
    }
    else
    {
        vdesc = (Vdesc *) realloc((char *)vdesc,
				  (unsigned) (num_vdescs + 1) * sizeof(Vdesc));
        vehicles_entries = (char **) realloc((char *)vehicles_entries,
					     (unsigned) (num_vdescs+1) *
					                sizeof(char *));
    }

    assert(vdesc != NULL);
    assert(vehicles_entries != NULL);

    /* If name present, load into that slot, else load into next empty slot */
    for (i = 0; i < num_vdescs; i++)
	if (!strcmp(name, vdesc[i].name))
	    break;
    d = &vdesc[i];
    retval = load_vdesc(d, name);

    /* Increment the number if a vdesc was loaded in the last slot */
    if (retval == DESC_LOADED && i == num_vdescs)
	num_vdescs++;
    *num = i;
    reset_dynamic_entries();
    return retval;
}

#define check_range(val,mx) \
  if((int)(val) < 0 || (int)(val) >= (int)(mx)) return DESC_BAD_FORMAT;

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

    for (i = 0; i < d->num_weapons; i++)
    {
	(void) fscanf(file, "%d %d", &weapon, &mount);
        check_range((int)weapon, MAX_WEAPON_STATS);

	switch (mount) {
	  case MOUNT_TURRET1:
	    if (num_tur < 1) return DESC_BAD_FORMAT;
	    break;
	  case MOUNT_TURRET2:
	    if (num_tur < 2) return DESC_BAD_FORMAT;
	    break;
	  case MOUNT_TURRET3:
	    if (num_tur < 3) return DESC_BAD_FORMAT;
	    break;
	  case MOUNT_FRONT: case MOUNT_BACK: case MOUNT_LEFT: case MOUNT_RIGHT:
	    break;
	  default:
	    return DESC_BAD_FORMAT;
	}

	d->weapon[i] = weapon;
	d->mount[i] = mount;
    }

    (void) fscanf(file, "%d", &d->armor.type);
    check_range(d->armor.type, MAX_ARMORS);

    for (i = 0; i < MAX_SIDES; i++)
    {
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

/*
** Saves the specified vehicle description.
** Returns one of DESC_NOT_FOUND, DESC_SAVED.
*/
save_vdesc(d)
Vdesc *d;
{
    FILE *file;
    char filename[MAXPATHLEN];
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

    (void) strcat(filename, d->name);
    (void) strcat(filename, ".v");
    if ((file = fopen(filename, "w")) == NULL)
	return DESC_NOT_FOUND;

    /* Save all the fields in the vdesc structure */
    (void) fprintf(file, "%s\n", d->name);
    (void) fprintf(file, "%s\n", d->designer);
    (void) fprintf(file, "%d\n", d->body);
    (void) fprintf(file, "%d\n", d->engine);
    (void) fprintf(file, "%d\n", d->num_weapons);
    for (i = 0; i < d->num_weapons; i++)
	(void) fprintf(file, "%d %d\n", d->weapon[i], d->mount[i]);

    (void) fprintf(file, "%d\n", d->armor.type);
    for (i = 0; i < MAX_SIDES; i++)
	(void) fprintf(file, "%d\n", d->armor.side[i]);

    (void) fprintf(file, "%d\n", d->specials);
    (void) fprintf(file, "%d\n", d->heat_sinks);

    (void) fprintf(file, "%d\n", d->suspension);
    (void) fprintf(file, "%d\n", d->treads);
    (void) fprintf(file, "%d\n", d->bumpers);

    (void) fclose(file);

#ifdef UNIX
    /* Change protections so others can read the vehicle in */
    (void) chmod(filename, 0644);
#endif

    return DESC_SAVED;
}

/*
** Loads the mdesc from the appropriate file into the next empty slot.
** Puts the number of the maze into num if loaded.
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT, DESC_NO_ROOM.
*/
make_mdesc(name, num)
char *name;
int *num;
{
    Mdesc *d;
    int retval, i;

    /* if the maze could not be found, we will be leaving room for an extra
       Mdesc.  This should probably be cleaned up later. */
    if (mdesc == NULL)
    {
	mdesc = (Mdesc *) malloc(2 * sizeof(Mdesc));
	mazes_entries = (char **) malloc(2 * sizeof(char *));
    }
    else
    {
        mdesc = (Mdesc *) realloc((char *)mdesc,
				  (unsigned) ((num_mdescs + 2) *
					      sizeof(Mdesc)));
        mazes_entries = (char **) realloc((char *)mazes_entries,
					  (unsigned) ((num_mdescs + 2) *
						      sizeof(char *)));
    }

    assert(mdesc != NULL);
    assert(mazes_entries != NULL);

    /* If name present, load into that slot, else load into next empty slot */
    for (i = 0; i < num_mdescs; i++)
	if (!strcmp(name, mdesc[i].name))
	    break;
    d = &mdesc[i];
    retval = load_mdesc(d, name);

    /* Increment the number if a mdesc was loaded in the last slot */
    if (retval == DESC_LOADED && i == num_mdescs)
	num_mdescs++;
    *num = i;
    reset_dynamic_entries();
    return retval;
}

/*
** Loads the maze description from the file "[mazesdir]/[name].m".
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT.
*/
load_mdesc(d, name)
Mdesc *d;
char *name;
{
    FILE *file;
    char filename[MAXPATHLEN];
    int temp;

#ifdef UNIX
    (void) strcpy(filename, pathname);
    (void) strcat(filename, "/");
    (void) strcat(filename, mazesdir);
    (void) strcat(filename, "/");
#endif				/* UNIX */

#ifdef AMIGA
    (void) strcpy(filename, "XVDIR:");
#endif				/* AMIGA */

    (void) strcat(filename, name);
    (void) strcat(filename, ".m");
    if ((file = fopen(filename, "r")) == NULL)
	return DESC_NOT_FOUND;

    if ((temp = getc(file)) == EOF)
	return DESC_BAD_FORMAT;
    d->type = (Game) temp;
    if (alloc_str(file, &d->name) == DESC_BAD_FORMAT)
	return DESC_BAD_FORMAT;
    if (alloc_str(file, &d->designer) == DESC_BAD_FORMAT)
	return DESC_BAD_FORMAT;
    if (alloc_str(file, &d->desc) == DESC_BAD_FORMAT)
	return DESC_BAD_FORMAT;
    if (alloc_str(file, (char **) &d->data) == DESC_BAD_FORMAT)
	return DESC_BAD_FORMAT;

    convert_maze(d, TO_INTERNAL_TYPE);

    (void) fclose(file);

    return DESC_LOADED;
}

/* At most 3 bytes per box in the grid for a maze description */
#define MAX_DATA_BYTES    GRID_WIDTH * GRID_HEIGHT * 3 + 1

/*
** Reads in a string from the file.  Mallocs the appropriate amount
** of memory for str, and copies the string into str.
** Returns DESC_BAD_FORMAT if string is too long or EOF is reached.
** Otherwise, returns DESC_LOADED.
*/
alloc_str(file, strp)
FILE *file;
char **strp;
{
    int ret;
    char temp[MAX_DATA_BYTES];
    unsigned int len;

    /* Read chars from file into temp up to and including the first '\0' */
    len = 0;
    do
    {
	if ((ret = getc(file)) == EOF || len >= MAX_DATA_BYTES)
	    return DESC_BAD_FORMAT;
	temp[len++] = (char) ret;
    } while ((char) ret != '\0');

    *strp = malloc(len);
    assert(*strp != NULL);
    (void) strcpy(*strp, temp);

    return DESC_LOADED;
}

/*
** Saves the specified maze description.
** Returns one of DESC_NOT_FOUND, DESC_SAVED.
*/
save_mdesc(d)
Mdesc *d;
{
	FILE *file;
	char filename[MAXPATHLEN];

#ifdef UNIX
	(void) strcpy(filename, pathname);
	(void) strcat(filename, "/");
	(void) strcat(filename, mazesdir);
	(void) strcat(filename, "/");
#endif							/* UNIX */

#ifdef AMIGA
	(void) strcpy(filename, "XVDIR:");
#endif							/* AMIGA */

	(void) strcat(filename, d->name);
	(void) strcat(filename, ".m");
	if ((file = fopen(filename, "w")) == NULL)
		return DESC_NOT_FOUND;
	
	convert_maze(d, TO_EXTERNAL_TYPE);

	(void) fputc((char) d->type, file);
	fputs(d->name, file);
	(void) fputc('\0', file);
	fputs(d->designer, file);
	(void) fputc('\0', file);
	fputs(d->desc, file);
	(void) fputc('\0', file);
	fputs((char *) d->data, file);
	(void) fputc('\0', file);

	(void) fclose(file);

#ifdef UNIX
	/* Change protections so others can read in the maze */
	(void) chmod(filename, 0644);
#endif

	return DESC_SAVED;
}

load_sdesc(d, name)
Sdesc *d;
char *name;
{
	FILE *file;
	char filename[MAXPATHLEN];

#ifdef UNIX
	(void) strcpy(filename, pathname);
	(void) strcat(filename, "/");
	(void) strcat(filename, mazesdir);
	(void) strcat(filename, "/");
#endif							/* UNIX */

#ifdef AMIGA
	(void) strcpy(filename, "XVDIR:");
#endif							/* AMIGA */

	(void) strcat(filename, name);
	(void) strcat(filename, ".s");
	if ((file = fopen(filename, "r")) == NULL)
		return DESC_NOT_FOUND;

/*
	if ((temp = getc(file)) == EOF)
		return DESC_BAD_FORMAT;
	d->type = (Game) temp;
	if (alloc_str(file, &d->name) == DESC_BAD_FORMAT)
		return DESC_BAD_FORMAT;
	if (alloc_str(file, &d->designer) == DESC_BAD_FORMAT)
		return DESC_BAD_FORMAT;
	if (alloc_str(file, &d->desc) == DESC_BAD_FORMAT)
		return DESC_BAD_FORMAT;
	if (alloc_str(file, (char **) &d->data) == DESC_BAD_FORMAT)
		return DESC_BAD_FORMAT;

	convert_maze(d, TO_INTERNAL_TYPE);
*/

	(void) fclose(file);

	return DESC_LOADED;
}

/*
** Loads the sdesc from the appropriate file into the next empty slot.
** Puts the number of the setup into num if loaded.
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT, DESC_NO_ROOM.
*/
/*ARGSUSED*/
make_sdesc(name, num)
char *name;
int *num;
{

    return (DESC_NOT_FOUND);
}


#ifdef UNIX
/*
** Reads environment variables for the pathname, username,
** vehicles directory, mazes directory, and programs directory.
*/
get_environment()
{
	extern char *getenv();
	char *p;

	/* Read in variables, providing appropriate defaults if not found */
	strcpy(username, (p = getenv("USER")) ? p :
		   ((p = getenv("LOGNAME")) ? p : "user"));
	strcpy(pathname, (p = getenv("XTANK_DIR")) ? p : XTANK_DIR);
	strcpy(vehiclesdir, (p = getenv("XTANK_VEHICLES")) ? p : "Vehicles");
	strcpy(mazesdir, (p = getenv("XTANK_MAZES")) ? p : "Mazes");
	strcpy(programsdir, (p = getenv("XTANK_PROGRAMS")) ? p : "Programs");
	if ((p = getenv("XTANK_HEADERS")) != NULL) {
		strcpy(headersdir, p);
	} else {
		strcpy(headersdir, pathname);
		strcat(headersdir, "/Src/Include");
	}
	strcpy(displayname, (p = getenv("DISPLAY")) ? p : "unix:0.0");

#ifdef NEED_AUX_FONT
	strcpy(fontdir, (p = getenv("FONT_DIR")) ? p : "Fonts");
#endif
}

#endif							/* UNIX */

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef AMIGA
#include <dos.h>
#endif

/*
** Reads a file, returning its contents in a null-terminated string.
** Returns an empty string if the file is not found.
*/
char *read_file(filename)
char *filename;
{
    FILE *file;
    char *string, temp[MAXPATHLEN];
    long size;

#ifdef AMIGA
    strcpy(temp, "XVDIR:");
#endif

#ifdef UNIX
    if (filename[0] != '/')
    {
	strcpy(temp, pathname);
	strcat(temp, "/");
    }
    else
	temp[0] = '\0';
#endif

    strcat(temp, filename);

    file = fopen(temp, "r");
    if (file == (FILE *) NULL)
    {
	char *empty;

	empty = malloc(1);	/* JMO 1/19/90 is free()'ed later */
	*empty = '\0';
	return (empty);
    }
    /* Get file size */

#ifdef AMIGA
    {
	struct FILEINFO finfo;

	if (dfind(&finfo, temp, 0))
	    return ("");
	size = finfo.fib_Size;
    }
#endif				/* AMIGA */

#ifdef UNIX
    {
	struct stat fileinfo;

	if (fstat(fileno(file), &fileinfo))
	{
	    char *empty;

	    empty = malloc(1);	/* It get's fred'ed JMO - 1/19/90 */
	    *empty = '\0';
	    return (empty);
	}
	size = fileinfo.st_size;
    }
#endif				/* UNIX */

    /* Leave space for the NULL */
    string = malloc((unsigned)size + 1);

    /* Copy the file into memory and close the file */
    (void) fread(string, sizeof(char), (int)size, file);
    (void) fclose(file);

    /* Put NULL at end of string */
    string[size] = '\0';

    return (string);
}

#ifdef SAVEIN

/* this needs work, it's obsolete */

int save_settings()
{
    int ctri, ctrj;
    char *dispname;
    FILE *outfile, *fopen();
    Video *vidptr;
    Vehicle *vptr;
    extern char *teams_entries[], *games_entries[];
    extern Terminal *terminal[];
    extern int num_terminals;

    outfile = fopen("xtank.settings", "w");
    if (!outfile)
    {
	/* error */
    }
    else
    {
	/* Settings */
	/* Maze */
	fprintf(outfile, "%s\n", settings.mdesc->name);

	/* Game */
	fprintf(outfile, "%s\n", games_entries[settings.game]);

	/* FLAGS */
	fprintf(outfile, "%d ", settings.point_bullets);
	fprintf(outfile, "%d ", settings.ricochet);
	fprintf(outfile, "%d ", settings.rel_shoot);
	fprintf(outfile, "%d ", settings.no_wear);
	fprintf(outfile, "%d ", settings.restart);
	fprintf(outfile, "%d ", settings.commentator);
	fprintf(outfile, "%d ", settings.full_map);
	fprintf(outfile, "%d ", settings.si.pay_to_play);
	fprintf(outfile, "%d ", settings.robots_dont_win);
	fprintf(outfile, "%d\n", settings.max_armor_scale);
#ifdef NONAMETAGS
	fprintf(outfile, "%d\n", settings.si.no_nametags);
#endif

	/* other settings */
	fprintf(outfile, "%d\n", settings.winning_score);
	fprintf(outfile, "%d\n", settings.outpost_strength);
	fprintf(outfile, "%f\n", settings.scroll_speed);
	fprintf(outfile, "%f\n", settings.box_slowdown);
	fprintf(outfile, "%f\n", settings.disc_friction);
	fprintf(outfile, "%f\n", settings.owner_slowdown);
	fprintf(outfile, "%f\n", settings.slip_friction);
	fprintf(outfile, "%f\n", settings.normal_friction);
	fprintf(outfile, "%d\n", settings.difficulty);

	fprintf(outfile, "%d\n", num_veh_alive);

	/* Information per vehicle */
	for (ctri = 0; ctri < num_veh_alive; ctri++)
	{
	    vptr = live_vehicles[ctri];

	    dispname = "NONE";
	    for (ctrj = 0; ctrj < num_terminals; ctrj++)
	    {
		if (terminal[ctrj]->vehicle == vptr)
		{
		    vidptr = terminal[ctrj]->video;
		    dispname = vidptr->display_name);
	    }
	}

	/* Vehicle # */
	fprintf(outfile, "V#%d:  ", vptr->number);

	/* Combatant */
	fprintf(outfile, "%s, ", vptr->owner->name);

	/* Display */
	fprintf(outfile, "%s, ", dispname);

	/* Vehicle */
	fprintf(outfile, "%s, ", vptr->name);

	/* Team */
	fprintf(outfile, "%s, ", teams_entries[vptr->team]);

	/* Programs */
	assert(vptr->num_programs == vptr->owner->num_programs);

	/* Num Programs */
	fprintf(outfile, "%d, ", vptr->owner->num_programs);

	if (vptr->num_programs)
	{
	    for (ctrj = 0; ctrj < vptr->num_programs; ctrj++)
	    {
		fprintf(outfile, "%s", vptr->program[ctrj]->desc->name);
		if (ctrj != vptr->num_programs - 1)
		    fprintf(outfile, ", ");
	    }
	}
	else
	{
	    fprinf(outfile, "NONE");
	}

	fprintf(outfile, "\n");
    }				/* end of for - for each vehicle */
}
}

#endif
