/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** program.c
*/

/*
$Author: aahz $
$Id: program.c,v 2.26 1992/09/12 09:42:26 aahz Exp $

$Log: program.c,v $
 * Revision 2.26  1992/09/12  09:42:26  aahz
 * added support for forcing specials
 *
 * Revision 2.25  1992/09/06  21:15:02  lidl
 * removed console initization stuff that the new console driver doesn't
 * use
 *
 * Revision 2.24  1992/05/15  03:59:04  lidl
 * removed warning messages from the SVR4 threading code
 *
 * Revision 2.23  1992/03/31  21:45:50  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.22  1992/02/13  05:05:12  aahz
 * changed gnat5 to gnat
 *
 * Revision 2.21  1992/02/13  03:47:23  aahz
 * changed gnat_prog to gnat5_prog.
 *
 * Revision 2.20  1992/02/06  09:01:11  aahz
 * set statically linked robot's filename to NULL
 *
 * Revision 2.19  1992/02/01  22:21:08  lidl
 * added gnat as one of the compiled in programs
 *
 * Revision 2.18  1992/01/30  03:43:20  aahz
 * removed ifdefs around no radar
 *
 * Revision 2.17  1992/01/29  08:37:01  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.16  1992/01/03  05:51:27  aahz
 * *** empty log message ***
 *
 * Revision 2.15  1991/12/27  02:16:09  lidl
 * changed to used compile-flag defines, debugging info for SVR4 boxen
 *
 * Revision 2.14  1991/12/16  02:54:24  lidl
 * changed the MP_OR_SUNLWP flag to THREADING_DEFINED
 * added check for the THREAD_SVR4 code
 *
 * Revision 2.13  1991/12/15  23:51:42  stripes
 * Added find_pdesc.
 *
 * Revision 2.12  1991/12/10  03:50:39  senft
 * Added tagman. Kurt added bootlegger.
 *
 * Revision 2.11  1991/12/03  19:57:09  stripes
 * changes to allow robots to run every frame (KJL)
 *
 * Revision 2.10  1991/09/30  01:25:44  lidl
 * removed warbuddy from list of programs that get compiled in
 *
 * Revision 2.9  1991/09/24  14:09:39  lidl
 * added RacerX_prog (from rpotter@grip.cis.upenn.edu)
 * bugfix for Full_Map from rpotter@grip.cis.upenn.edu
 *
 * Revision 2.8  1991/09/20  09:04:22  lidl
 * added Guard and warbuddy to the list of robots that get loaded automatically
 *
 * Revision 2.7  1991/09/19  06:48:15  lidl
 * added three new robots -- Pzwk_I is way cool!  Ran code through indent.
 *
 * Revision 2.6  1991/09/18  06:47:40  stripes
 * Added 4 new programs.
 *
 * Revision 2.5  1991/09/15  06:55:27  stripes
 * checked in for 1.2g++ release
 *
 * Revision 2.4  1991/03/25  00:42:11  stripes
 * RS6K Patches (IBM is a rock sucker)
 *
 * Revision 2.3  1991/02/10  13:51:30  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:47  rpotter
 * complete rewrite of vehicle death, other tweaks
 *
 * Revision 2.1  91/01/17  07:12:45  rpotter
 * lint warnings and a fix to update_vector()
 *
 * Revision 2.0  91/01/17  02:10:21  rpotter
 * small changes
 *
 * Revision 1.1  90/12/29  21:02:59  aahz
 * Initial revision
 *
*/

#include "malloc.h"
#include <assert.h>
#include "xtanklib.h"
#include "xtank.h"
#include "thread.h"
#include "icounter.h"
#include "vehicle.h"
#include "globals.h"
#include "graphics.h"


extern int frame;
extern Vehicle *cv;
extern Map real_map;
extern Settings settings;


/* Time (in microseconds) a program is allotted per frame (on average) */
#define PROGRAM_ALLOT 5000

/* maximum time (in microseconds) a program is allowed to run on any given
   frame */
#define PROGRAM_TIME 10000

/* stack space programs get */
#define THREAD_TOTALSIZE (sizeof(Thread) + STACK_SIZE)


/* Clock values to time robot program execution */
Clock current_time, end_time;

/* Pointer to the program scheduler's thread */
Thread *scheduler_thread;

Prog_desc *prog_desc[MAX_PDESCS];
int num_prog_descs = 0;

/*
** Changes the current vehicle pointer to the specified vehicle.
** All xtanklib functions act on the current vehicle.
*/
set_current_vehicle(v)
Vehicle *v;
{
    cv = v;
}

/*
** Initializes the array of program descriptions at startup time.
*/
init_prog_descs()
{
	int iCtr;
	extern Prog_desc
	kamikaze_prog, drone_prog, warrior_prog, shooter_prog, eliza_prog,
	Buddy_prog, Flipper_prog, artful_prog, spot_prog, Diophantine_prog,
	hud3_prog, Dio_prog,			/* New with 1.2g & not all that tested: */
	Pzkw_I_prog, dum_maze_prog, roadrunner_prog,
	Guard_prog, RacerX_prog,		/* New with 1.3b & extensively tested */
	tagman_prog, Bootlegger_prog, gnat_prog;

    num_prog_descs = 0;
    prog_desc[num_prog_descs++] = &kamikaze_prog;
    prog_desc[num_prog_descs++] = &drone_prog;
    prog_desc[num_prog_descs++] = &warrior_prog;
    prog_desc[num_prog_descs++] = &shooter_prog;
    prog_desc[num_prog_descs++] = &eliza_prog;
    prog_desc[num_prog_descs++] = &Buddy_prog;
    prog_desc[num_prog_descs++] = &Flipper_prog;
    prog_desc[num_prog_descs++] = &Diophantine_prog;
    prog_desc[num_prog_descs++] = &artful_prog;
    prog_desc[num_prog_descs++] = &spot_prog;
    prog_desc[num_prog_descs++] = &hud3_prog;
    prog_desc[num_prog_descs++] = &Dio_prog;
    prog_desc[num_prog_descs++] = &Pzkw_I_prog;
    prog_desc[num_prog_descs++] = &dum_maze_prog;
    prog_desc[num_prog_descs++] = &roadrunner_prog;
    prog_desc[num_prog_descs++] = &Guard_prog;
    prog_desc[num_prog_descs++] = &RacerX_prog;
    prog_desc[num_prog_descs++] = &tagman_prog;
    prog_desc[num_prog_descs++] = &Bootlegger_prog;
    prog_desc[num_prog_descs++] = &gnat_prog;

	for (iCtr = 0; iCtr < num_prog_descs; iCtr++)
	{
		prog_desc[iCtr]->filename = (char *)0;
	}

}

/*
** Initializes the threader and signal handler at the beginning of a battle.
*/
init_threader()
{
    /* Initialize threading system */
    scheduler_thread = thread_setup();

    /* how 'bout some error checking here? */

#ifdef THREADING_DEFINED
    syntax error - if this define is used, choose another
#endif

#ifdef THREAD_MP
#define THREADING_DEFINED
#endif

#ifdef THREAD_SUNLWP
#define THREADING_DEFINED
#endif

#ifdef THREAD_SVR4
#define THREADING_DEFINED
#endif

#ifdef THREADING_DEFINED
    if (!scheduler_thread)
	 rorre("malloc failed in thread_setup");

#endif


    /* Initialize the itimer signal handler */
    setup_counter();
}


init_specials(v)
Vehicle *v;
{
    int i, j;
    Mapper *m = (Mapper *) v->special[(int) MAPPER].record;
	extern char force_states[];

/*
    for (i = 0; i < MAX_SPECIALS; i++)
    {
        if (force_states[i] == INT_FORCE_OFF)
        {
            v->special[i].status = SP_nonexistent;
        }
		else
        if (force_states[i] == INT_FORCE_ON)
        {
            v->special[i].status = SP_off;
        }
    }
*/

    if (settings.si.no_radar) {
        v->special[(int) RADAR].status = SP_nonexistent;
        v->special[(int) NEW_RADAR].status = SP_nonexistent;
    }

    /* Copy maze into mapper if full_map is on */
    if (v->special[(int) MAPPER].status != SP_nonexistent &&
	    settings.si.full_map) {
	Landmark_info *s;

	/* copy map */
	bcopy((char *) real_map, (char *) m->map, sizeof(real_map));

	/* Initialize landmarks */
	m->num_landmarks = 0;
	for (i = 0; i < GRID_WIDTH; i++)
	    for (j = 0; j < GRID_HEIGHT; j++) {
		if (m->map[i][j].type != NORMAL &&
			m->num_landmarks < MAX_LANDMARKS) {
		    s = &m->landmark[m->num_landmarks++];
		    s->type = m->map[i][j].type;
		    s->team = m->map[i][j].team;
		    s->x = i;
		    s->y = j;
		}
	    }
    } else {			/* clear out any old map memories */
	if (m)
	    bzero((char *) m, sizeof(*m));
    }

    /* Activate the specials */
    for (i = 0; i < MAX_SPECIALS; i++) {
	if (v->special[i].status != SP_nonexistent) {
	    v->special[i].status = SP_off;	/* otherwise activation fails */
	    do_special(v, (SpecialType) i, SP_activate);
	}
    }
}


/*
 * This is an unused stub, but I plan to use it in the
 * future. It might be wired up now, I'm not sure.
 */
 
zap_specials(v)
Vehicle *v;
{
    int i;
    for (i = 0; i < MAX_SPECIALS; i++) {
	if (v->special[i].status == SP_on)
	  ;  /* do_special(v, (SpecialType) i, SP_deactivate); */
    }
}


/*
** Initializes all programs for the specified vehicle.
*/
init_programs(v)
Vehicle *v;
{
    Program *prog;
    int i;

    for (i = 0; i < v->num_programs; i++) {
	prog = &v->program[i];

	if (prog->thread_buf == NULL) {
	    /* Allocate the memory for the thread and stack */
	    if ((prog->thread_buf = malloc(THREAD_TOTALSIZE)) == NULL)
		rorre("init_programs(): malloc failed");
	}
	assert(prog->thread == NULL);
	prog->thread = (char *) thread_init(prog->thread_buf, THREAD_TOTALSIZE,
					(Thread * (*) ()) prog->desc->func);

#ifdef THREADING_DEFINED
	if (prog->thread == NULL)
	    rorre("thread_init() failed in init_programs()");
#endif

	/* programs shouldn't run the first frame, since mapper has not
	   received its initial update by then */
	prog->total_time = (frame + 1) * PROGRAM_ALLOT;
	prog->status = PROG_on;

	prog->next_message = 0;	/* no new messages yet */
	prog->cleanup = NULL;	/* no cleanup function yet */
    }
}

/*
** Runs all the programs that should be run during a frame of execution.
*/
run_all_programs()
{
    extern frame;
    Vehicle *v;
    Program *prog;
    int i, j;

    /* For every vehicle, run all of its programs */
    for (i = 0; i < num_veh_alive; i++) {
	v = live_vehicles[i];

	/* Set the current vehicle pointer to this vehicle for xtanklib */
	set_current_vehicle(v);

	/* Run all the programs for this vehicle */
	for (j = 0; j < v->num_programs; j++) {
	    /* Set the current program in the vehicle */
	    prog = v->current_prog = &v->program[j];

	    /* If the prog hasn't used its alloted time, run it */
	    if (prog->total_time <= frame * PROGRAM_ALLOT)
		run_program(prog);

	    /* Nullify the current program pointer once the program is
	       finished */
	    v->current_prog = (Program *) NULL;
	}
    }
}

/*
** Runs the specified program and returns the amount of time it took to run.
*/
int run_program(prog)
Program *prog;
{
    /* Start up interval counter to time programs */
    clear_clock();
    write_clock(&end_time, PROGRAM_TIME);
    start_counter();

    /* Call the function */
    thread_switch((Thread *) prog->thread);

    /* Stop the interval timer */
    stop_counter();

    /* Add the elapsed program time to the total time for the prog */
    prog->total_time += read_clock(&current_time);

    /* Return the elapsed program time */
    return read_clock(&current_time);
}

/*
** If the elapsed time is more than the amount of time a program
** is supposed to have, check_time switches control back to the scheduler.
** If there is no current program, or the current program has not run
** for long enough, check_time does nothing.
*/
check_time()
{
    /* If there is a program running */
    if (cv->current_prog != (Program *) NULL) {
	/* If the program has been running too long, stop it */
	get_clock(&current_time);
	if (compare_clocks(&current_time, &end_time) == -1)
	    thread_switch(scheduler_thread);
    }
}

/*
** Stops the current program, upping elapsed_prog_time to PROGRAM_ALLOT.
*/
stop_program()
{
    Clock temp;

    if (cv->current_prog != (Program *) NULL) {
	/* Up the elapsed time to the given amount */
	write_clock(&temp, PROGRAM_ALLOT);
	get_clock(&current_time);
	if (compare_clocks(&current_time, &temp) == 1) {
	    write_clock(&current_time, PROGRAM_ALLOT);
	}
	thread_switch(scheduler_thread);	/* Stop the program */
    } else {
	printf("stop_program(): no program??\n");
    }
}

/*
** Makes the specified programs for the specified vehicle.
*/
make_programs(v, num_progs, prog_num)
Vehicle *v;
int num_progs;
int prog_num[];
{
    int num, i;

    /* Insert the proper program descriptions into the program array */
    for (i = 0; i < num_progs; i++) {
	/* Make sure we aren't making too many programs */
	if (v->num_programs == MAX_PROGRAMS)
	    break;

	/* Make sure the program number is reasonable */
	num = prog_num[i];
	if (num < 0 || num >= num_prog_descs)
	    continue;

	/* Copy the program description pointer into the program */
	v->program[v->num_programs++].desc = prog_desc[num];
    }
}

int find_pdesc(prog_name, index_return)
int *index_return;
char *prog_name;
{
	int i;

	for(i = 0; i < num_prog_descs; i++) {
		if (!prog_desc[i] || strcmp(prog_name, prog_desc[i]->name)) continue;

		*index_return = i;
		return DESC_LOADED;
	}

	return DESC_NOT_FOUND;
}
