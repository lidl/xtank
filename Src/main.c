#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** main.c
*/

#include "config.h"
#include "xtank.h"
#include "bullet.h"


#ifdef DEBUG
#define debug(str) puts(str)
#else
#define debug(str)
#endif


char *mode_str[] = {"Single", "Multi", "Demo", "Battle", "Menu"};
char *game_str[] = {"Combat", "War", "Ultimate", "Capture", "Race"};
char *bool_str[] = {"False", "True"};
char team_char[] = "NROYGBV";

Eset actual_eset, *eset;
Bset actual_bset, *bset;

Maze maze;
Map box;
Settings settings;
Boolean game_paused;
int frame;						/* the game clock */
int num_vehicles;
Vehicle actual_vehicle[MAX_VEHICLES], *vehicle[MAX_VEHICLES];

char *executable_name;



main(argc, argv)
int argc;
char *argv[];

{
	extern int num_terminals;
	extern char username[], displayname[], video_error_str[];
	int ret, i;

	bset = &actual_bset;
	eset = &actual_eset;

#ifdef UNIX
	{
		extern char *network_error_str[];
		extern char *malloc();

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
#endif							/* UNIX */

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

	/* Initialize pointers to vehicles */
	debug("Initializing vehicle pointers");
	for (i = 0; i < MAX_VEHICLES; i++)
		vehicle[i] = &actual_vehicle[i];

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
}
