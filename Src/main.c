/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** main.c
*/

/*
$Author: lidl $
$Id: main.c,v 2.5 1991/09/15 09:24:51 lidl Exp $

$Log: main.c,v $
 * Revision 2.5  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.4  1991/03/25  00:42:11  stripes
 * RS6K Patches (IBM is a rock sucker)
 *
 * Revision 2.3  1991/02/10  13:51:06  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:23  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:12  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:57  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:43  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "bullet.h"


#ifdef DEBUG
#define debug(str) puts(str)
#else
#define debug(str)
#endif


char *game_str[] = {"Combat", "War", "Ultimate", "Capture", "Race"};
char team_char[] = "NROYGBV";

Eset actual_eset, *eset;
Bset actual_bset, *bset;

Maze maze;
Map real_map;
Settings settings;
Boolean game_paused;
int frame;			/* the game clock */
char *executable_name;

/****************************************************************************\
* A little terminology: a "dead" vehicle is one that is waiting a few frames *
* to be resurrected.  Robot threads that belonged to dead vehicles are	     *
* deallocated, but the Vehicle structure itself is kept.  The entry in	     *
* live_vehicles[] is moved to dead_vehicles[], though.			     *
* 									     *
* When a vehicle is out of the game for good (like if the restart setting is *
* off), it becomes "permanently dead" and its Vehicle structure is	     *
* deallocated.  Terminals that belonged to such vehicles are left in	     *
* observation mode, and robot threads are deallocacted.			     *
\****************************************************************************/

int num_veh;			/* number of vehicles in the game (some living,
				   some dead) */
int num_veh_alive;		/* number of currently living vehicles */
int num_veh_dead;		/* number of currently dead vehicles */
Vehicle actual_vehicles[MAX_VEHICLES];	/* up to "num_veh" entries are
					   valid (permanently dead vehicles
					   leave holes) */
Vehicle *live_vehicles[MAX_VEHICLES];	/* pointers into actual_vehicles[], the
					   first "num_veh_alive" entries are
					   valid */
Vehicle *dead_vehicles[MAX_VEHICLES];	/* pointers into actual_vehicles[], the
					   first "num_veh_dead" entries are
					   valid */


void debugger_break()
{
    ;				/* this is a good place to but a breakpoint */
}


int main(argc, argv)
    int argc;
    char *argv[];
{
    extern int num_terminals;
    extern char displayname[], video_error_str[];
    int ret, i;

    bset = &actual_bset;
    eset = &actual_eset;

#ifdef UNIX
    {
	extern char *network_error_str[];
#if !defined(_IBMR2)
	extern char *malloc();
#endif

	/* Get environment variables */
	debug("Getting environment variables");
	get_environment();
        executable_name = malloc((unsigned) strlen(argv[0]) + 1);
	(void) strcpy(executable_name, argv[0]);

        /* If there are multiple display names, check to make sure all displays
	   are on same subnet, to avoid producing gateway traffic. */
	if (argc > 1)
	{
	    debug("Checking internet addresses");
	    ret = check_internet(argc - 1, argv + 1);
	    if (ret)
	    {
		fprintf(stderr, network_error_str[ret]);
		fprintf(stderr, "\n");
		if (ret == 5)
		{
		    fprintf(stderr, "You can't play xtank with terminals that are far\n");
		    fprintf(stderr, "away from each other, since the entire network\n");
		    fprintf(stderr, "would slow down.\n");
		}
		rorre("Cannot continue.");
	    }
	}
    }
#endif				/* UNIX */

    /* Initialize various and sundry things */
    debug("Initializing various things");
    init_random();
    init_prog_descs();
    init_vdesign();
    init_settings();
    init_threader();
    init_msg_sys();

    /* Rotate vehicle objects */
    debug("Rotating vehicle objects");
    rotate_objects();

    /* Open the graphics toolkit */
    debug("Opening graphics toolkit");
    open_graphics();

    /* Parse command line for display names and make a terminal for each one */
    debug("Making terminals");
    if (argc > 1)
    {
	for (i = 0; i < argc - 1; i++)
	    if (make_terminal(argv[i + 1]))
		rorre(video_error_str);
    }
    else if (make_terminal(displayname))
	rorre(video_error_str);

    if (num_terminals == 0)
	rorre("No terminals opened.  Cannot continue program.");

    /* Load descriptions after determining num_vehicle_objs */
    debug("Loading vehicle and maze descriptions");
    load_desc_lists();

    /* Ask each terminal for player name and vehicle name */
    debug("Getting player info");
    for (i = 0; i < num_terminals; i++)
    {
	set_terminal(i);
	get_player_info();
    }

    /* Go to main interface */
    debug("Entering main interface");
    main_interface();

    /* Close up shop in a friendly way */
    free_everything();

    return(0);
}
