#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** program.c
*/

#include <assert.h>
#include "xtank.h"
#include "thread.h"
#include "icounter.h"
#include "vehicle.h"


extern char *malloc();
extern int frame;
extern Vehicle *cv;


/* Time (in microseconds) a program is allowed to run */
#define PROGRAM_TIME 10000

/* Time (in microseconds) a program is allotted per frame */
#define PROGRAM_ALLOT 5000

/* Size of buffer put after each Thread for stack space */
#define THREAD_BUFSIZE (25 * 1024)


static int kludge = TRUE;

/* Clock values to time robot program execution */
Clock current_time, end_time;

/* Pointer to the program_scheduler's thread */
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
	 kamikaze_prog, drone_prog, warrior_prog, shooter_prog, eliza_prog,
	 Buddy_prog, Flipper_prog, artful_prog, spot_prog, Diophantine_prog,
	 hud_prog, Dio_prog;

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
	prog_desc[num_prog_descs++] = &hud_prog;
	prog_desc[num_prog_descs++] = &Dio_prog;
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

/*
** Initializes all programs for the specified vehicle.
*/
init_programs(v)
Vehicle *v;
{
	Program *prog;
	int i;
	char *ptr;
	unsigned int size;

	/* Initialize one thread for each program in the vehicle */
	for (i = 0; i < v->num_programs; i++)
	{
		prog = &v->program[i];

		/* Allocate the memory for the thread and buffer */
		/* THREAD_BUFSIZE is the stack size for the new thread GHS */
		size = sizeof(Thread) + THREAD_BUFSIZE;
        ptr = malloc(size);     
        if (!ptr)               
            rorre("malloc failed in init_programs");    
        prog->thread = (char *) thread_init(ptr, size,
					    (Thread *(*)())prog->desc->func);

#ifdef MP_OR_SUNLWP
        if (!prog->thread)      
            rorre("thread_init failed in init_programs");       
#endif

        /* Programs run every other frame, try to make about half run each
		   frame Don't have any of them run the first frame, since mapper has
		   not received its initial update by then. */
		prog->total_time = (frame + 1 + rnd(2)) * PROGRAM_ALLOT;
		prog->status = PROG_on;

        /* No new messages at start so next message in desc should be cleared
		   */
		prog->next_message = 0;
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
	extern int num_vehicles;
	extern Vehicle *vehicle[];
	Vehicle *v;
	Program *prog;
	int i, j;

	/* For every vehicle, run all of its programs */
	for (i = 0; i < num_vehicles; i++)
	{
		v = vehicle[i];

		/* Set the current vehicle pointer to this vehicle for xtanklib */
		set_current_vehicle(v);

		/* Run all the programs for this vehicle */
		for (j = 0; j < v->num_programs; j++)
		{
			/* Set the current program in the vehicle */
			prog = v->current_prog = &v->program[j];
			kludge = FALSE;

			/* If the prog hasn't used its alloted time, run it */
			if (prog->total_time <= frame * PROGRAM_ALLOT)
				run_program(prog);

			/* Nullify the current program pointer once the program is
			   finished */
			v->current_prog = (Program *) NULL;
			kludge = TRUE;
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
	if (cv->current_prog != (Program *) NULL)
	{
		/* If the program has been running too long, stop it */
		get_clock(&current_time);
		if (compare_clocks(&current_time, &end_time) == -1)
			thread_switch(scheduler_thread);
	}
	else
	{
		assert(kludge);
	}
}

/*
** Stops the current program, upping elapsed_prog_time to PROGRAM_TIME.
*/
stop_program()
{
	Clock temp;

	if (cv->current_prog != (Program *) NULL)
	{
		/* Up the elapsed time to the given amount */
		write_clock(&temp, PROGRAM_TIME);
		get_clock(&current_time);
		if (compare_clocks(&current_time, &temp) == 1)
			write_clock(&current_time, PROGRAM_TIME)
			/* Stop the program */
				thread_switch(scheduler_thread);
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
	for (i = 0; i < num_progs; i++)
	{
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
