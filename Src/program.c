/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** program.c
*/

/*
$Author: lidl $
$Id: program.c,v 2.10 1991/09/30 01:25:44 lidl Exp $

$Log: program.c,v $
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


/* Time (in microseconds) a program is allowed to run */
#define PROGRAM_TIME 10000

/* Time (in microseconds) a program is allotted per frame */
#define PROGRAM_ALLOT 5000

/* stack space programs get */
#define THREAD_STACKSIZE (25 * 1024)
#define THREAD_TOTALSIZE (sizeof(Thread) + THREAD_STACKSIZE)


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
    extern Prog_desc
     kamikaze_prog, drone_prog, warrior_prog, shooter_prog, eliza_prog, Buddy_prog,
     Flipper_prog, artful_prog, spot_prog, Diophantine_prog, hud3_prog,
     Dio_prog,			/* New with 1.2g & not all that tested: */
     Pzkw_I_prog, dum_maze_prog, roadrunner_prog,
     Guard_prog, RacerX_prog;

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
}

/*
** Initializes the threader and signal handler at the beginning of a battle.
*/
init_threader()
{
    /* Initialize threading system */
    scheduler_thread = thread_setup();

    /* how 'bout some error checking here? */

#ifdef MP_OR_SUNLWP
    syntax error - if this define is used, choose another
#endif

#ifdef THREAD_MP
#define MP_OR_SUNLWP
#endif

#ifdef THREAD_SUNLWP
#define MP_OR_SUNLWP
#endif

#ifdef MP_OR_SUNLWP
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

    if (v->special[CONSOLE].status != SP_nonexistent) {
	/* initialize console */
	for (j = 0; j < MAX_ENTRIES; j++)
	    ((Console *) (v->special[CONSOLE].record))->_entry[j].color = WHITE;
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

#ifdef MP_OR_SUNLWP
	if (prog->thread == NULL)
	    rorre("thread_init() failed in init_programs()");
#endif

	/* Programs run every other frame; try to make about half run each
	   frame.  Don't have any of them run the first frame, since mapper
	   has not received its initial update by then. */
	prog->total_time = (frame + 1 + rnd(TICKSZ)) * PROGRAM_ALLOT;
	prog->status = PROG_on;

	prog->next_message = 0;	/* no new messages yet */
	prog->cleanup = NULL;	/* no cleanup function yet */
    }
}

/*
** Runs all the programs that should be run during a frame of execution.
** On average, a program is executed PROGRAM_ALLOT microseconds per frame.
** It simulates this by running each program for PROGRAM_TIME
** microseconds once every PROGRAM_TIME/PROGRAM_ALLOT frames.
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
** Stops the current program, upping elapsed_prog_time to PROGRAM_TIME.
*/
stop_program()
{
    Clock temp;

    if (cv->current_prog != (Program *) NULL) {
	/* Up the elapsed time to the given amount */
	write_clock(&temp, PROGRAM_TIME);
	get_clock(&current_time);
	if (compare_clocks(&current_time, &temp) == 1) {
	    write_clock(&current_time, PROGRAM_TIME);
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
