/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** program.c
*/

#include "xtank.h"
#include "thread.h"
#include "icounter.h"
#include "program.h"

/* Pointer to the program_scheduler's thread */
Thread *scheduler_thread;

extern char *malloc();

Prog_desc *prog_desc[MAX_PDESCS];
int num_prog_descs = 0;

extern Vehicle *cv;

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
  extern Prog_desc drone_prog,warrior_prog,shooter_prog,eliza_prog,buddy_prog;

  prog_desc[num_prog_descs++] = &drone_prog;
  prog_desc[num_prog_descs++] = &warrior_prog;
  prog_desc[num_prog_descs++] = &shooter_prog;
  prog_desc[num_prog_descs++] = &eliza_prog;
  prog_desc[num_prog_descs++] = &buddy_prog;
}

/*
** Initializes the threader and signal handler at the beginning of a battle.
*/
init_threader()
{
  /* Initialize threading system */
#ifdef notdef
  scheduler_thread =(Thread*)malloc((unsigned)(sizeof(Thread)+THREAD_BUFSIZE));
  thread_setup(scheduler_thread,THREAD_BUFSIZE);
#else
  thread_setup();
  scheduler_thread = mpthd_me();
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

  /* Initialize one thread for each program in the vehicle */
  for(i = 0 ; i < v->num_programs ; i++) {
    prog = &v->program[i];

    /* Allocate the memory for the thread and buffer */
    prog->thread = (char *) malloc((unsigned)(sizeof(Thread)+THREAD_BUFSIZE));
    thread_init((Thread *) prog->thread,THREAD_BUFSIZE,prog->desc->func);

    /*
    ** Programs run every other frame, try to make about half run each frame
    ** Don't have any of them run the first frame, since mapper has not
    ** received its initial update by then.
    */
    prog->total_time = (frame + 1 + rnd(2)) * PROGRAM_ALLOT;
    prog->status = PROG_on;

    /* No new messages at start so next message in desc should be cleared */
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
  int i,j;

  /* For every vehicle, run all of its programs */
  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];

    /* Set the current vehicle pointer to this vehicle for xtanklib */
    set_current_vehicle(v);

    /* Run all the programs for this vehicle */
    for(j = 0 ; j < v->num_programs ; j++) {
      /* Set the current program in the vehicle */
      prog = v->current_prog = &v->program[j];

      /* If the prog hasn't used its alloted time, run it */
      if(prog->total_time <= frame * PROGRAM_ALLOT)
	run_program(prog);

      /* Nullify the current program pointer once the program is finished */
      v->current_prog = (Program *) NULL;
    }
  }
}

Clock current_time,end_time;

/*
** Runs the specified program and returns the amount of time it took to run.
*/
int run_program(prog)
     Program *prog;
{
  /* Start up interval counter to time programs */
  clear_clock();
  write_clock(&end_time,PROGRAM_TIME);
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
  if(cv->current_prog != (Program *) NULL) {
    /* If the program has been running too long, stop it */
    get_clock(&current_time);
    if (compare_clocks(&current_time,&end_time) == -1)
      thread_switch(scheduler_thread);
  }
}

/*
** Stops the current program, upping elapsed_prog_time to PROGRAM_TIME.
*/
stop_program()
{
  Clock temp;

  if(cv->current_prog != (Program *) NULL) {
    /* Up the elapsed time to the given amount */
    write_clock(&temp,PROGRAM_TIME);
    get_clock(&current_time);
    if(compare_clocks(&current_time,&temp) == 1)
      write_clock(&current_time,PROGRAM_TIME)

    /* Stop the program */
    thread_switch(scheduler_thread);
  }
}

/*
** Makes the specified programs for the specified vehicle.
*/
make_programs(v,num_progs,prog_num)
     Vehicle *v;
     int num_progs;
     int prog_num[];
{
  int num,i;

  /* Insert the proper program descriptions into the program array */
  for(i = 0 ; i < num_progs ; i++) {
    /* Make sure we aren't making too many programs */
    if(v->num_programs == MAX_PROGRAMS) break;

    /* Make sure the program number is reasonable */
    num = prog_num[i];
    if(num < 0 || num >= num_prog_descs) continue;

    /* Copy the program description pointer into the program */
    v->program[v->num_programs++].desc = prog_desc[num];
  }
}
