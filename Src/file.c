/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** file.c
*/

/*
$Author: lidl $
$Id: file.c,v 2.27 1992/06/07 02:45:08 lidl Exp $

$Log: file.c,v $
 * Revision 2.27  1992/06/07  02:45:08  lidl
 * Post Adam Bryant patches and a manual merge of the rejects (ugh!)
 *
 * Revision 2.26  1992/04/09  04:16:46  lidl
 * hmmm, old copy seems to be damaged.  Only changes should have been
 * to use tanklimits.h and not limits.h
 *
 * Revision 2.25  1992/03/31  04:04:16  lidl
 * pre-aaron patches, post 1.3d release (ie mailing list patches)
 *
 * Revision 2.24  1992/02/06  09:00:37  aahz
 * added save Program info in the save_settings file.
 *
 * Revision 2.23  1992/02/04  08:25:08  lidl
 * removed ifdef of new vehicle ending letter code
 *
 * Revision 2.22  1992/02/04  07:25:33  aahz
 * init retcode to desc_loaded rather than a hard-coded 0
 *
 * Revision 2.21  1992/02/03  07:31:09  lidl
 * the new load code now can be linked, but core-dumps when it attempts
 * to load a new (.V) file.  Checking in so that others can play with the code
 *
 * Revision 2.20  1992/01/30  03:41:21  aahz
 * removed ifdefs around no radar
 *
 * Revision 2.19  1992/01/29  08:37:01  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.18  1992/01/21  03:44:24  aahz
 * reorganized load_vdesc to make loading of multiple formats
 * easier.
 *
 * Revision 2.17  1992/01/21  02:36:03  stripes
 * Added call to save vechiles in new format.
 *
 * Revision 2.16  1992/01/02  02:53:12  aahz
 * added a filename parameter to save_settings
 *
 * Revision 2.15  1991/12/20  21:11:24  lidl
 * made char *games_entries[]; an extern, so as to mollify ANSI C compilers
 *
 * Revision 2.14  1991/12/19  05:38:24  stripes
 * changed delimter from space to tab (Kurt as Josh)
 *
 * Revision 2.13  1991/12/15  20:27:36  lidl
 * a small SVR4 compatibility hack
 *
 * Revision 2.12  1991/12/02  10:31:45  lidl
 * changed to handle a forth turret on tanks
 *
 * Revision 2.11  1991/12/02  06:38:23  lidl
 * changed to use limits.h, and got rid of the foolish things
 *
 * Revision 2.10  1991/11/27  06:26:13  aahz
 * added team score to save setup.
 *
 * Revision 2.9  1991/11/15  04:11:53  stripes
 * Stuff in save_settings, I think it is done.
 *
 * Revision 2.8  1991/10/07  06:15:33  stripes
 * Added a save_settings(), it saves to /tmp/something-or-other
 *
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

#include "tanklimits.h"
#include "malloc.h"
#include <assert.h>
#include "xtank.h"
#include "graphics.h"
#include "mazeconv.c"
#include "gr.h"
#include "vdesc.h"
#include "setup.h"
#include "vehicle.h"
#include "terminal.h"
#include "globals.h"
#ifdef UNIX
#include <sys/param.h>
#include <sys/dir.h>
#endif
#if defined(SVR4) || defined(SYSV)
#include <dirent.h>
#endif

char pathname[MAXPATHLEN];
char headersdir[MAXPATHLEN];	/* full name of directory to find headers in */
char vehiclesdir[MAXNAMLEN], mazesdir[MAXNAMLEN], programsdir[MAXNAMLEN];
char username[MAX_STRING], displayname[256];

#ifdef NEED_AUX_FONT
char fontdir[MAXNAMLEN];
#endif

extern char *games_entries[];
extern Settings settings;

extern int num_prog_descs;
extern Prog_desc *prog_desc[];

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

    /* If name present, load into that slot, else load into next empty slot */
    for (i = 0; i < num_vdescs; i++)
	{
		if (!strcmp(name, vdesc[i].name))
		{
	    	break;
		}
	}

	if (i == num_vdescs)
	{
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
	}

    d = &vdesc[i];
    retval = load_vdesc(d, name);

    /* Increment the number if a vdesc was loaded in the last slot */
    if (retval == DESC_LOADED && i == num_vdescs)
	{
		num_vdescs++;
	}
    *num = i;
    reset_dynamic_entries();
    return retval;
}

#define check_range(val,mx) \
  if((int)(val) < 0 || (int)(val) >= (int)(mx)) return DESC_BAD_FORMAT;

int ReadVehicleFormat0(file, d)
	FILE *file;
	Vdesc *d;
{
	int i;
    int num_tur;
	int iRetCode = DESC_LOADED;
    WeaponType weapon;
    MountLocation mount;

	/* Initialize max side to 0 */
	d->armor.max_side = 0;

    /* Load the values into the vdesc structure, checking their validity */
    (void) fscanf(file, "%s", d->name);
    (void) fscanf(file, "%s", d->designer);

    (void) fscanf(file, "%d", &d->body);
    check_range(d->body, num_vehicle_objs);
    num_tur = vehicle_obj[d->body]->num_turrets;

	if (num_tur == 4)
	{
		iRetCode = DESC_BAD_FORMAT;
	}
	else
	{
    	(void) fscanf(file, "%d", &d->engine);
    	check_range(d->engine, MAX_ENGINES);

    	(void) fscanf(file, "%d", &d->num_weapons);
    	check_range(d->num_weapons, MAX_WEAPONS + 1);

    	for (i = 0; i < d->num_weapons; i++)
    	{
			(void) fscanf(file, "%d %d", &weapon, &mount);
        	check_range((int)weapon, MAX_WEAPON_STATS);

			if (mount >= MOUNT_TURRET4)
			{
				++mount;
			}

			switch (mount)
			{
	  			case MOUNT_TURRET1:
	    			if (num_tur < 1)
					{
						iRetCode = DESC_BAD_FORMAT;
					}
	    			break;
	  			case MOUNT_TURRET2:
	    			if (num_tur < 2)
					{
						iRetCode = DESC_BAD_FORMAT;
					}
	    			break;
	  			case MOUNT_TURRET3:
	    			if (num_tur < 3)
					{
						iRetCode = DESC_BAD_FORMAT;
					}
	    			break;
	  			case MOUNT_FRONT:
				case MOUNT_BACK:
				case MOUNT_LEFT:
				case MOUNT_RIGHT:
	    			break;
	  			default:
	    			iRetCode = DESC_BAD_FORMAT;
					break;
			}

			if (iRetCode)
			{
				break;
			}

			d->weapon[i] = weapon;
			d->mount[i] = mount;
    	}
	}

	if (iRetCode == 0)
	{
    	(void) fscanf(file, "%d", &d->armor.type);
    	check_range(d->armor.type, MAX_ARMORS);

    	for (i = 0; i < MAX_SIDES; i++)
    	{
			(void) fscanf(file, "%d", &d->armor.side[i]);
			if (d->armor.max_side < d->armor.side[i])
			{
	    		d->armor.max_side = d->armor.side[i];
			}
    	}

    	(void) fscanf(file, "%d", &d->specials);
    	(void) fscanf(file, "%d", &d->heat_sinks);

    	(void) fscanf(file, "%d", &d->suspension);
    	check_range(d->suspension, MAX_SUSPENSIONS);

    	(void) fscanf(file, "%d", &d->treads);
    	check_range(d->treads, MAX_TREADS);

    	(void) fscanf(file, "%d", &d->bumpers);
    	check_range(d->bumpers, MAX_BUMPERS);
	}

	return (iRetCode);
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
    char filename[MAXPATHLEN];
    int i;
	int iFormatType;
	int iRetCode = DESC_LOADED;

	/* initialize the vdesc struct */
	memset(d, 0, sizeof(Vdesc));

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
    (void) strcat(filename, ".V");

	if ((file = fopen(filename, "r")) == NULL) {
		filename[strlen(filename) - 1] = 'v';
		if ((file = fopen(filename, "r")) == NULL) {
			iRetCode = DESC_NOT_FOUND;
		} else {
			iFormatType = 0;
		}
	} else {
		iFormatType = 1;
	}

	if (iRetCode == 0) {
		switch (iFormatType) {
			case 0:
				iRetCode = ReadVehicleFormat0(file, d);
				break;

			case 1:
				iRetCode = ReadVehicleFormat1(file, d);
				break;
		}

		if (iRetCode == 0) {
    		/* Compute parameters, and see if there are any problems */
				if (compute_vdesc(d)) {
					iRetCode = DESC_BAD_FORMAT;
			}
		}
	}

	if (file) {
		(void) fclose(file);
	}

	return (iRetCode);
}


/*
** Saves the specified vehicle description.
** Returns one of DESC_NOT_FOUND, DESC_SAVED.
*/
save_vdesc(d)
Vdesc *d;
{
	return (SaveVehicleFormat1(d));
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

    /* If name present, load into that slot, else load into next empty slot */
    for (i = 0; i < num_mdescs; i++)
	{
		if (!strcmp(name, mdesc[i].name))
		{
	    	break;
		}
	}

	if (i == num_mdescs)
	{
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
	}

    d = &mdesc[i];
    retval = load_mdesc(d, name);

    /* Increment the number if a mdesc was loaded in the last slot */
    if (retval == DESC_LOADED && i == num_mdescs)
	{
		num_mdescs++;
	}
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

int save_settings(pcFileName)
	char *pcFileName;
{
    int ctri, ctrj;
	int iProg;
    char *dispname;
	char *ProgWritten;
    FILE *outfile, *fopen();
    Video *vidptr;
    Vehicle *vptr;
	Prog_desc *pdesc;
    extern char *teams_entries[];
    extern Terminal *terminal[];
    extern int num_terminals;

    outfile = fopen(pcFileName, "w");
	assert(outfile);

	fprintf(outfile, "xtank: 114336\n");
	/* Settings */
	/* Maze */
	fprintf(outfile, "Maze: %s\n", (settings.mdesc) ? settings.mdesc->name
		: "Random");

	/* Game */
	fprintf(outfile, "Game: %s\n", games_entries[settings.si.game]);

	/* FLAGS */
	fprintf(outfile, "Point Bullets: %d\n", settings.point_bullets);
	fprintf(outfile, "Ricochet: %d\n", settings.si.ricochet);
	fprintf(outfile, "Rel Fire: %d\n", settings.si.rel_shoot);
	fprintf(outfile, "No Wear: %d\n", settings.si.no_wear);
	fprintf(outfile, "Restart: %d\n", settings.si.restart);
	fprintf(outfile, "Commentator: %d\n", settings.commentator);
	fprintf(outfile, "Full Map: %d\n", settings.si.full_map);
	fprintf(outfile, "Pay To Play: %d\n", settings.si.pay_to_play);
	fprintf(outfile, "Robots Don't Win: %d\n", settings.robots_dont_win);
	fprintf(outfile, "Max Armor Scale: %d\n", settings.max_armor_scale);
	fprintf(outfile, "Nametags: %d\n", settings.si.no_nametags);
	fprintf(outfile, "No Radar: %d\n", settings.si.no_radar);
	fprintf(outfile, "Team Score: %d\n", settings.si.team_score);
	fprintf(outfile, "Player Teleport: %d\n", settings.si.player_teleport);
	fprintf(outfile, "Disc Teleport: %d\n", settings.si.disc_teleport);
	fprintf(outfile, "Teleport From Team: %d\n", settings.si.teleport_from_team);
	fprintf(outfile, "Teleport From Neutral: %d\n", settings.si.teleport_from_neutral);
	fprintf(outfile, "Teleport To Team: %d\n", settings.si.teleport_to_team);
	fprintf(outfile, "Teleport To Neutral: %d\n", settings.si.teleport_to_neutral);
	fprintf(outfile, "Teleport Any To Any: %d\n", settings.si.teleport_any_to_any);
	fprintf(outfile, "War Goals Only: %d\n", settings.si.war_goals_only);
	fprintf(outfile, "Relative Disc: %d\n", settings.si.relative_disc);
	fprintf(outfile, "Ultimate Own Goal: %d\n", settings.si.ultimate_own_goal);

	/* other settings */
	fprintf(outfile, "Winning Score: %d\n", settings.si.winning_score);
	fprintf(outfile, "Outpost Strength: %d\n", settings.si.outpost_strength);
	fprintf(outfile, "Scroll Speed: %f\n", settings.si.scroll_speed);
	fprintf(outfile, "Box slowdown: %f\n", settings.si.box_slowdown);
	fprintf(outfile, "Disc Friction: %f\n", settings.si.disc_friction);
	fprintf(outfile, "Disc Speed: %f\n", settings.si.disc_speed);
	fprintf(outfile, "Disc Damage: %f\n", settings.si.disc_damage);
	fprintf(outfile, "Disc Heat: %f\n", settings.si.disc_heat);
	fprintf(outfile, "Owner Slowdown: %f\n", settings.si.owner_slowdown);
	fprintf(outfile, "Slip Friction: %f\n", settings.si.slip_friction);
	fprintf(outfile, "Normal Friction: %f\n", settings.si.normal_friction);
	fprintf(outfile, "Shocker Walls: %d\n", settings.si.shocker_walls);
	fprintf(outfile, "Difficulty: %d\n", settings.difficulty);

	customized_combatants();
	init_combatants();
	setup_game(False);

	ProgWritten = calloc(num_prog_descs, sizeof(char));
	assert(ProgWritten);

	fprintf(outfile, "Tanks: %d\n", num_veh_alive);

	/* Information per vehicle */
	for (ctri = 0; ctri < num_veh_alive; ctri++) {
		vptr = live_vehicles[ctri];

		/* join the players grid's programs with the prog_desc dynamic progs */
		/* and write the result */
		if (vptr->num_programs)
		{
			for (ctrj = 0; ctrj < vptr->num_programs; ctrj++) 
			{
				for (iProg = 0; iProg < num_prog_descs; iProg++)
				{
					pdesc = prog_desc[iProg];

					/* if it is the same struct */
					if (vptr->program[ctrj].desc == pdesc)
					{
						/* if the program is dynamically loaded */
						if (pdesc->filename != (char *)0)
						{
							/* if I have not already written it */
							if (ProgWritten[iProg] == False)
							{
								fprintf(outfile, "Program:\t%s\t%s\n",
										pdesc->name, pdesc->filename);

								ProgWritten[iProg] = True;
							}
						}
					}
				}
			}
		}

		dispname = "NONE";
		for (ctrj = 0; ctrj < num_terminals; ctrj++) {
			if (terminal[ctrj]->vehicle == vptr) {
				vidptr = (Video *)terminal[ctrj]->video;
				dispname = vidptr->display_name;
			}
		}

		/* Vehicle # */
		fprintf(outfile, "V#%d:\t", vptr->number);

		/* Combatant */
		fprintf(outfile, "%s,\t", vptr->owner->name);

		/* Display */
		fprintf(outfile, "%s,\t", dispname);

		/* Vehicle */
		fprintf(outfile, "%s,\t", vptr->name);

		/* Team */
		fprintf(outfile, "%s,\t", teams_entries[vptr->team]);

		/* Programs */
		assert(vptr->num_programs == vptr->owner->num_programs);

		/* Num Programs */
		fprintf(outfile, "%d,\t", vptr->owner->num_programs);

		if (vptr->num_programs)
		{
			for (ctrj = 0; ctrj < vptr->num_programs; ctrj++) {
				fprintf(outfile, "%s", vptr->program[ctrj].desc->name);
				if (ctrj != vptr->num_programs - 1) fprintf(outfile, ",\t");
			}
		} else {
			fprintf(outfile, "NONE");
		}

		fprintf(outfile, "\n");
	}				/* end of for - for each vehicle */

	fclose(outfile);
}
